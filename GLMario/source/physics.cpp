#include "physics.h"

#include "utility.h"
#include <assert.h>
#include "renderer.h"
#include "mathops.h"
#include "input.h"

Rectf CanonicalRect(DynamicCollider* col)
{
    Rectf result = {};
    result.x = col->position.x + col->rect.x;
    result.y = col->position.y + col->rect.y;
    result.w = col->rect.w;
    result.h = col->rect.h;

    return result;
}

#ifdef _DEBUG
std::vector<Rectf> Physics::minkowski_rects;
#endif

Physics::Physics()
: statics()
, dynamics()
{
    memset(&this->quadtree, 0, sizeof(this->quadtree));
}

Physics::~Physics()
{
}

RArrayRef<StaticCollider> Physics::AddStaticCollider(Rectf r)
{
    StaticCollider col;
    col.active = true;
    col.rect = r;
    col.rotation = TAU / 8.f; //TAU / 4.f;

// Fill in the aabb
    RotatedRect(col.rect, col.rotation, &col.aabb);

    return AddStaticCollider(col);
}

RArrayRef<StaticCollider> Physics::AddStaticCollider(StaticCollider col)
{
    auto result = statics.Add(col);
    AddCollider(&this->quadtree, &this->quadtree_memory, result.RawPointer());
    return result;
}

RArrayRef<DynamicCollider> Physics::AddDynamicCollider(DynamicCollider col)
{
	return dynamics.Add(col);
}

void Physics::DestroyCollider(RArrayRef<DynamicCollider> col)
{
    if(dynamics.IsValid(col))
    {
        dynamics.Remove(col);
    }
}

#ifdef _DEBUG
void Physics::AddMinkowskiDebugRect(Rectf rect)
{
	Physics::minkowski_rects.push_back(rect);
}
#endif

void Physics::DebugDraw()
{
    // call this after "step" so that the arrays are already sorted
    Renderer* ren = Renderer::get();

    Vec4 s_active = vec4(1, 0, 0, 1);
    Vec4 s_inactive = s_active;
    s_inactive.a = 0.5f;
    Vec4 d_active = vec4(0.3f, 0.3f, 1, 1);
    Vec4 d_inactive = d_active;
    d_inactive.a = 0.5f;


    LineDrawParams params;
    params.line_width = 3;

    for(uint32 i = 0; i < statics.Size(); ++i)
    {
        ren->DrawRotatedRect(statics[i].rect, statics[i].rotation, statics[i].active ? s_active : s_inactive, &params);
    }

    for(uint32 i = 0; i < dynamics.Size(); ++i)
    {
        ren->DrawRect(CanonicalRect(&dynamics[i]), dynamics[i].active ? d_active : d_inactive, &params);
    }

    DrawBoundingBoxes(&this->quadtree, ren);
}

#if 0 // Fix the CheckCollision call and reenable this
bool Physics::RaycastStatics(Vec2 start, Vec2 cast, CollisionInfo& outHit, bool draw)
{
    bool result = false;
    float closest = Length(cast);

    Rectf rect = { 0 };
    rect.bot = start.y;
    rect.left = start.x;
    rect.width = 0;
    rect.height = 0;

    for(uint32 i = 0; i < statics.Size(); ++i)
    {
        StaticCollider& scol = statics[i];

		if (!scol.active) continue;

        CollisionInfo ci;
        if (CheckCollision(rect, cast, scol.rect, scol.rotation, ci))
        {
            result = true;
            if(ci.distance < closest)
            {
                closest = ci.distance;

                outHit.normal = ci.normal;
                outHit.projection = ci.projection;
				outHit.distance = closest;
				outHit.point = ci.point;
            }
        }
    }

    return result;
}
#endif

Vec2 Physics::StepCollider(MemoryArena* temporary_memory, RArrayRef<DynamicCollider> refCollider, Vec2& velocity, float dt)
{
    ProfileBeginSection(Profile_PhysicsStepCollider);
    assert(dynamics.IsValid(refCollider));
    DynamicCollider& col = (*refCollider.ptr);// dynamics[refCollider.index];
    col.collisions.resize(0);

    Vec2 startPosition = col.position;
    Vec2 startVelocity = velocity;

    Vec2 remainingVelocity = velocity * dt;
    CollisionInfo closestCollisionInfo = {};
	closestCollisionInfo.distance = Length(remainingVelocity);
    float closestCollision = Length(remainingVelocity);
    bool collided = false;

    CollisionInfo ci;
    const int maxIterations = 5;
    int iterations = 0;

    float maxX = 0;
    float maxY = 0;

	bool valid = true;

    StaticCollider** potential_colliders_base;
    PushArrayScoped(potential_colliders_base, &this->quadtree_memory, StaticCollider*, statics.Size());

    // Find the potentially colliding statics
    Rectf col_plus_velocity = CanonicalRect(&col);
    col_plus_velocity.w += abs(remainingVelocity.x);
    col_plus_velocity.h += abs(remainingVelocity.y);
    if(remainingVelocity.x < 0)
    {
        col_plus_velocity.x += remainingVelocity.x;
    }
    if(remainingVelocity.y < 0)
    {
        col_plus_velocity.y += remainingVelocity.y;
    }

    StaticCollider** potential_colliders = potential_colliders_base;
    uint32 potential_colliders_size = 0;
    StaticCollider** end = GetPotentialColliders(&this->quadtree, col_plus_velocity, potential_colliders, &potential_colliders_size);
    assert((size_t) end == (size_t)potential_colliders + potential_colliders_size * sizeof(potential_colliders[0]));

    CollisionInfo* detected_collisions_base;
    PushArrayScoped(detected_collisions_base, temporary_memory, CollisionInfo, potential_colliders_size);
    CollisionInfo* detected_collisions = detected_collisions_base;
    uint32 detected_collisions_count = 0;

// Generate minkowski sums and aabbs against the dynamic collider
    Vec2_8* minkowski_sums;
    PushArrayScoped(minkowski_sums, temporary_memory, Vec2_8, potential_colliders_size);
    // TODO: First pass Contain check against the aabb
    Rectf* minkowski_aabbs;
    PushArrayScoped(minkowski_aabbs, temporary_memory, Rectf, potential_colliders_size);
    for(uint32 i = 0; i < potential_colliders_size; ++i)
    {
        // Assume no rotation on dynamic collider
        minkowski_sums[i] = MinkowskiSum(potential_colliders[i]->rect, potential_colliders[i]->rotation,
                                        col.rect, 0, &minkowski_aabbs[i]);
    }

    Rectf* aligned_minkowski_sums;
    PushArrayScoped(aligned_minkowski_sums, temporary_memory, Rectf, potential_colliders_size);
    for(uint32 i = 0; i < potential_colliders_size; ++i)
    {
        aligned_minkowski_sums[i] = MinkowskiSum(potential_colliders[i]->rect, col.rect);
    }

    Renderer* ren = Renderer::get();

// TODO: Get rid of mSumOther since the sums are being stored throughout the function now
    do
    {
        bool found_hit = false;
        collided = false;
        memset(&ci, 0, sizeof(CollisionInfo));

#if 0
        Renderer* ren = Renderer::get();
        for(uint32 i = 0; i < collision_list_size; ++i)
        {
            ren->DrawRect(collision_list[i]->rect, 4, DrawLayer_Debug, { 1.f, 1.f, 0.5f, 1.f });
        }
#endif
        int32 num_detected = 0;

        uint32 detected_collisions_count = 0;

        for(uint32 j = 0; j < potential_colliders_size; ++j)
        {
            ProfileBeginSection(Profile_PhysicsInnerLoop);
            StaticCollider* scol = potential_colliders[j];
            if (!scol->active) continue;

            if (CheckCollision(CanonicalRect(&col), remainingVelocity, &minkowski_sums[j], ci))
			{
                ci.msum_index = j;
                ci.rotation_other = potential_colliders[j]->rotation;
				valid = true;

num_detected++;
#if 0  // Isn't this redundant with the next check?
                // Check penetration against previous collisions
                for (uint32 c = 0; c < detected_collisions_count; ++c)
                {
					if (Contains(potential_colliders[detected_collisions[c].msum_index]->rect, detected_collisions[c].rotation_other, ci.point))
					{
						valid = false;
						break;
					}
                }
#endif

                // Check penetration against potential colliders
                if(false)
                {
                    Rectf curRect = CanonicalRect(&col);
                    for(uint32 k = 0; k < potential_colliders_size; ++k)
                    {
                        if(Contains(minkowski_aabbs[k], ci.point))
                        {
                            if(Contains(aligned_minkowski_sums[k], potential_colliders[k]->rotation, ci.point))
                            {
                                //static int counter = 0;
                                //printf("%d Canceling because contains\n", counter++);
                                LineDrawParams params;
                                params.draw_layer = DrawLayer_Debug;
                                ren->DrawRotatedRect(aligned_minkowski_sums[k], potential_colliders[k]->rotation, vec4(0, 1.f, 0, 1.f));
                                valid = false;
                                found_hit = true;
                                break;
                            }
                        }
#if 0
                        Rectf* other = &potential_colliders[k]->rect;
                        if(!potential_colliders[k]->active) continue;

                        Rectf mSum = MinkowskiSum(*other, curRect);
                        if(Contains(demSum, ci.point))
                        {
                            valid = false;
                            break;
                        }
#endif
                    }
                }

                // Resolve collision if it is still valid
				if (valid)
				{
                    assert(detected_collisions_count <= potential_colliders_size);
                    detected_collisions[detected_collisions_count++] = ci;
					collided = true;

					if (closestCollision > ci.distance)
					{
						remainingVelocity = ci.projection;
						closestCollision = ci.distance;
						//maxX = MinSigned(maxX, ci.point.x - col.position.x);
						//maxY = MinSigned(maxY, ci.point.y - col.position.y);
						closestCollisionInfo = ci;

					}
                }
            }

            ProfileEndSection(Profile_PhysicsInnerLoop);
        }

        DebugPrintf("Detected: %d", num_detected);

        if (collided)
        {
            col.collisions.push_back(closestCollisionInfo);
            col.position = closestCollisionInfo.point;//  + ci.projection;

            remainingVelocity = ci.projection;

			float rh = 0.2f;
			float rw = rh * 2.f;
            const uint8 line_width = 3;
			// Renderer::get()->DrawRect(rectf(ci.point.x + ci.projection.x - rh, ci.point.y + ci.projection.y - rh, rw, rw), line_width, DrawLayer_UI, vec4(1, 1, 0, 1));

            // col.position.x = MinSigned(col.position.x, maxX);
            // col.position.y = MinSigned(col.position.y, maxY);
        }
		else if (valid)
        {
            if (found_hit)
            {
                printf("trouble\n");
            }
            col.position += remainingVelocity;
        }
		else
		{

		}

    }while(iterations++ < maxIterations && collided && Length(remainingVelocity) > COLLISION_EPSILON);


    // Just an assertion of curiousity. If this fires, there is nothing wrong, It just means that many
    // bounces are being handled.
    assert(iterations != maxIterations);

    velocity = col.position - startPosition;

    if (Length(velocity) < 0.0001f)
    {
        velocity.x = 0;
        velocity.y = 0;
    }
    else
    {
        velocity.x /= dt;
        velocity.y /= dt;
        velocity.x = sign(velocity.x) * min(abs(velocity.x), abs(startVelocity.x));
        velocity.y = sign(velocity.y) * min(abs(velocity.y), abs(startVelocity.y));
    }

    ProfileEndSection(Profile_PhysicsStepCollider);
    return col.position;
}

bool LineSegmentIntersection(Vec2 r0, Vec2 r1, Vec2 a, Vec2 b, Vec2& result)
{
    Vec2 s1, s2;
    s1 = r1 - r0;
    s2 = b - a;

    float s, t;
    s = (-s1.y * (r0.x - a.x) + s1.x * (r0.y - a.y)) / (-s2.x * s1.y + s1.x * s2.y);
    t = (s2.x * (r0.y - a.y) - s2.y * (r0.x - a.x)) / (-s2.x * s1.y + s1.x * s2.y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        // Return the point of intersection
        result = vec2(r0.x + (t * s1.x), r0.y + (t * s1.y));
        return true;
    }
    return false; // No collision
}

// NOTE(cgenova): Look up "Vector Projection" to shoot the ray along the wall after a collision
// TODO: SIMD
bool CheckCollision(const Rectf& m, Vec2 velocity, Vec2_8* mSum, CollisionInfo& out)
{
	bool result = false;
	Vec2 o = { m.x + m.w / 2.f, m.y + m.h / 2.f };

	Ray motion = { { o.x , o.y },
					{ o.x + velocity.x, o.y + velocity.y } };

    // Do the Minkowski sum
    //Vec2 center = rect_center(other); // unused ?
    //TODO:remove this mSum, just use the one from RotatedRectMinkowski
//    Rectf mSum = MinkowskiSum(other, m);

// change this msumother
    //out.mSumOther = mSum;
    //out.rotation_other = other_rot;

#if 0
    Rectf mink_aabb;
    const float m_rot = 0; // assuming no dynamic collider rotation for now
    Vec2_8 p = MinkowskiSum(other, other_rot, m, m_rot, &mink_aabb);
    ren->DrawRect(mink_aabb, vec4(1.f, 1.f, 1.f, 1.f));
#endif

    Renderer* ren = Renderer::get();

    Array<SimpleVertex> box;
    for(int i = 0; i < 8; ++i)
    {
        SimpleVertex v = { };
        v.color = vec4(0.2f, 0.2f, 1.f, 1.f);
        v.position  = mSum->e[i];
        box.Add(v);
    }
    LineDrawParams params;
    params.line_draw_flags |= LineDraw_Looped;
    ren->DrawLine(box, &params);

	Ray mRays[8] = {};
    for(int i = 0; i < 7; ++i)
    {
        mRays[i] = make_ray(mSum->e[i], mSum->e[i + 1]);
    }
    mRays[7] = make_ray(mSum->e[7], mSum->e[0]);

#if 0
    if(!Contains(mSum, other_rot, o))
    {
        printf("Collider inside of object!");
    }
#endif

    float closest = -1.f;

	for (int i = 0; i < 8; ++i)
	{
        Vec2 intersection;
        // TODO: SIMD
        if(LineSegmentIntersection(motion.v0, motion.v1, mRays[i].v0, mRays[i].v1, intersection))
        {
            float distance = Length(intersection - motion.v0);

            if(!result || distance < closest)
            {
                result = true;
                closest = distance;
                out.distance = distance;

                // Magnitude of projection vector
                float remainingDistanceToTravel = Length(velocity) - distance;

                Vec2 b = mRays[i].v1 - mRays[i].v0;
                Vec2 n = Normalize(vec2(-b.y, b.x));
                Vec2 d = motion.v1 - motion.v0;
                Vec2 r = d - (2.f * Dot(d, n) * n);

                r = Normalize(r);
                r *= remainingDistanceToTravel;

                out.projection = (Dot(r, b) / Dot(b, b)) * b;

                Vec2 geometryOffset = n * COLLISION_EPSILON;

                out.projection += geometryOffset * 1.1f;
                out.normal = n;
                if(distance > COLLISION_EPSILON)
                {
                    // out.point = lerp(o, intersection, 0.9f);
                    out.point = intersection + geometryOffset;
                }
                else
                {
                    out.point = o;
                }
            }
        }
	}
	return result;
}

/**********************************************
 *
 * Physics Quadtree
 *
 ***************/


bool Contains(PhysicsNode* physics_node, Rectf rect)
{
    bool result = Intersects(physics_node->aabb, rect);
    return result;
}

bool Contains(PhysicsNode* physics_node, Vec2 point)
{
    bool result = Contains(physics_node->aabb, point);
    return result;
}

bool IsLeaf(PhysicsNode* physics_node)
{
    bool result = !physics_node->is_parent;
    return result;
}

// NOTE: could switch to doing this by ID instead of pointer?
// @untested! We currently don't allow the removal of static colliders so this hasn't been used
uint32 RemoveCollider(PhysicsNode* node, StaticCollider* to_delete)
{
    uint32 num_removed = 0;

    if(IsLeaf(node))
    {
        for(uint8 i = 0; i < node->contained_colliders; ++i)
        {
            if(node->colliders[i] == to_delete)
            {
                num_removed++;
                auto remaining = (MAX_LEAF_SIZE - 1) - node->contained_colliders;
                if(remaining > 0)
                {
                    memmove(&node->colliders[i], &node->colliders[i + 1], sizeof((uint32)node->colliders[i] * remaining));
                    node->colliders[i + 1] = 0;
                }
                else
                {
                    node->colliders[i] = 0; // This is the last one in the array
                }
            }
        }
    }
    else
    {
        PhysicsNode* child_node = node->child_nodes;
        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i)
        {
            num_removed += RemoveCollider(child_node, to_delete);
        }
    }

    return num_removed;
}

void AddCollider(PhysicsNode* physics_node, MemoryArena* arena, StaticCollider* collider)
{
    if(IsLeaf(physics_node) && physics_node->contained_colliders < MAX_LEAF_SIZE)
    {
        physics_node->colliders[physics_node->contained_colliders] = collider;
        ++physics_node->contained_colliders;
    }
    else if(IsLeaf(physics_node))
    {

        // Split the node into 4
        StaticCollider* temp_colliders[MAX_LEAF_SIZE];
        memcpy(temp_colliders, physics_node->colliders, sizeof(temp_colliders));

        physics_node->child_nodes = PushArray(arena, PhysicsNode, QUADTREE_CHILDREN);
        physics_node->is_parent = true;

        uint16 new_depth = physics_node->depth + 1;
        PhysicsNode* new_node = physics_node->child_nodes;

        Rectf r = physics_node->aabb;
        float half_width = r.w / 2.f;
        float half_height = r.h / 2.f;

        Rectf divided_rects[QUADTREE_CHILDREN] = {
            { r.x              , r.y              , half_width, half_height },
            { r.x + half_height, r.y              , half_width, half_height },
            { r.x              , r.y + half_height, half_width, half_height },
            { r.x + half_height, r.y + half_height, half_width, half_height },
        };

        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++new_node)
        {
            new_node->aabb  = divided_rects[i];
            new_node->depth = new_depth;
        }

        // Add the original collider that was passed in.
        AddCollider(physics_node, arena, collider);

        // re-add the old pointers to the new child nodes.
        for(uint32 i = 0; i < MAX_LEAF_SIZE; ++i)
        {
            AddCollider(physics_node, arena, temp_colliders[i]);
        }
    }
    else
    { // Not a leaf, recurse
        PhysicsNode* child = physics_node->child_nodes;
        for(uint32 j = 0; j < QUADTREE_CHILDREN; ++j, ++child)
        {
            if(Contains(child, collider->aabb))
            {
                AddCollider(child, arena, collider);
            }
        }
    }
}

StaticCollider** GetPotentialColliders(PhysicsNode* node, Rectf aabb, StaticCollider** collision_list, uint32* list_size)
{
    StaticCollider** index = collision_list;

    if(Contains(node, aabb))
    {
        if(IsLeaf(node))
        {
            uint8 col_count = node->contained_colliders;
            if(col_count > 0)
            {
                size_t copy_size = col_count * sizeof(node->colliders[0]);
                memcpy(index, node->colliders, copy_size);

                index += col_count;
                *list_size += col_count;
            }
        }
        else
        {
            PhysicsNode* child = node->child_nodes;
            for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++child)
            {
                index = GetPotentialColliders(child, aabb, index, list_size);
            }
        }
    }
    return index;
}

void DrawBoundingBoxes(PhysicsNode* physics_node, Renderer* ren)
{
    const float blue  = (float)physics_node->depth / 10.f;
    const float green = Contains(physics_node, MouseWorldPosition()) ? 0.8f : 0.1f;

    Vec4 color = { 0, green, blue, 0.5f };

    //ren->DrawRect(physics_node->aabb, line_width, DrawLayer_UI, color, LineDrawOptions::CUSTOM_SIZE);

    if(!IsLeaf(physics_node))
    {
        PhysicsNode* node = physics_node->child_nodes;
        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++node)
        {
            DrawBoundingBoxes(node, ren);
        }
    }

    LineDrawParams params;
    params.line_width = 2;
    ren->DrawRect(physics_node->aabb, color, &params);
}

