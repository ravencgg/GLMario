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
    DrawLayer dl = DrawLayer_UI;

    Vec4 s_active = vec4(1, 0, 0, 1);
    Vec4 s_inactive = s_active;
    s_inactive.a = 0.5f;
    Vec4 d_active = vec4(0.3f, 0.3f, 1, 1);
    Vec4 d_inactive = d_active;
    d_inactive.a = 0.5f;

    const uint8 line_width = 3;

    for(uint32 i = 0; i < statics.Size(); ++i)
    {
        ren->DrawRect(statics[i].rect, line_width, dl, statics[i].active ? s_active : s_inactive);
    }

    for(uint32 i = 0; i < dynamics.Size(); ++i)
    {
        ren->DrawRect(CanonicalRect(&dynamics[i]), line_width, dl, dynamics[i].active ? d_active : d_inactive);
    }

#ifdef _DEBUG
	for (uint32 i = 0; i < Physics::minkowski_rects.size(); ++i)
    {
		ren->DrawRect(Physics::minkowski_rects[i], line_width, dl, { 0, 1, 0, 1 });
    }
	Physics::minkowski_rects.clear();
#endif

    DrawBoundingBoxes(&this->quadtree, ren);
}

bool Physics::RaycastStatics(Vec2 start, Vec2 cast, CollisionInfo& outHit, bool draw)
{
    bool result = false;
    float closest = length(cast);

    Rectf rect = { 0 };
    rect.top = start.y;
    rect.left = start.x;
    rect.width = 0;
    rect.height = 0;

    for(uint32 i = 0; i < statics.Size(); ++i)
    {
        StaticCollider& scol = statics[i];

		if (!scol.active) continue;

        CollisionInfo ci;
        if (CheckCollision(rect, cast, scol.rect, ci))
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


#if 0
void Physics::StepDynamicColliders(float dt)
{
	// NOTE(cgenova): Bubble sort would be faster here.
	std::sort(active_dynamics.begin(), active_dynamics.end());

	for (uint32 i = 0; i < active_dynamics.size(); ++i)
	{
		TDynamicCollider& col = dynamics[active_dynamics[i]];
		col.collisions.resize(0);

		Vec2 remainingVelocity = col.velocity;
		CollisionInfo closestCollisionInfo = {};
		float closestCollision = length(remainingVelocity);
		bool collided = false;

        for(uint32 j = 0; j < active_statics.size(); ++j)
        {
            TStaticCollider& scol = statics[active_statics[j]];
			if (!scol.active) continue;

            CollisionInfo ci;

			// TODO(cgenova): redo this loop while remainingVelocity is > 0, limited by iteration count.
			if (CheckCollision(col.rect, remainingVelocity, scol.rect, ci))
            {
				// This isn't right, this should only be for motion right now.
				collided = true;

				if (closestCollision > ci.distance)
				{
					closestCollision = ci.distance;
					closestCollisionInfo = ci;
				}
            }
        }
		if (collided)
		{
			col.collisions.push_back(closestCollisionInfo);
			col.position = closestCollisionInfo.point;
		}
    }
}
#endif

Vec2 Physics::StepCollider(RArrayRef<DynamicCollider> refCollider, Vec2& velocity, float dt)
{
    ProfileBeginSection(Profile_PhysicsStepCollider);
    assert(dynamics.IsValid(refCollider));
    DynamicCollider& col = (*refCollider.ptr);// dynamics[refCollider.index];
    col.collisions.resize(0);

    Vec2 startPosition = col.position;
    Vec2 startVelocity = velocity;

    Vec2 remainingVelocity = velocity * dt;
    CollisionInfo closestCollisionInfo = {};
	closestCollisionInfo.distance = length(remainingVelocity);
    float closestCollision = length(remainingVelocity);
    bool collided = false;

    CollisionInfo ci;
    const int maxIterations = 5;
    int iterations = 0;

    float maxX = 0;
    float maxY = 0;

	std::vector<CollisionInfo> collisions;
	bool valid = true;

    // TODO: end of scope macro to do PopAllocations
    StaticCollider** collision_list_base = PushArray(&this->quadtree_memory, StaticCollider*, statics.Size());

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

    StaticCollider** collision_list = collision_list_base;
    uint32 collision_list_size = 0;
    StaticCollider** end = GetPotentialColliders(&this->quadtree, col_plus_velocity, collision_list, &collision_list_size);
    assert((size_t) end == (size_t)collision_list + collision_list_size * sizeof(collision_list[0]));

    do
    {
        collided = false;
        memset(&ci, 0, sizeof(CollisionInfo));

#if 0
        Renderer* ren = Renderer::get();
        for(uint32 i = 0; i < collision_list_size; ++i)
        {
            ren->DrawRect(collision_list[i]->rect, 4, DrawLayer_Debug, { 1.f, 1.f, 0.5f, 1.f });
        }
#endif

        for(uint32 j = 0; j < collision_list_size; ++j)
        {
            ProfileBeginSection(Profile_PhysicsInnerLoop);
            StaticCollider* scol = collision_list[j];
            if (!scol->active) continue;

			if (CheckCollision(CanonicalRect(&col), remainingVelocity, scol->rect, ci))
			{
				valid = true;

				for (auto it : collisions)
				{
					if (Contains(it.mSumOther, ci.point))
					{
						valid = false;
						break;
					}
				}

                Rectf curRect = CanonicalRect(&col);
                for(uint32 k = 0; k < collision_list_size; ++k)
                {
					Rectf* other = &collision_list[k]->rect;
                    if(!collision_list[k]->active) continue;

                    Rectf mSum = { other->x - curRect.w / 2.f,
                            other->y - curRect.h / 2.f,
                            curRect.w + other->w,
                            curRect.h + other->h };

                    if(Contains(mSum, ci.point))
                    {
                        valid = false;
                        break;
                    }
                }

				if (valid)
				{
					collisions.push_back(ci);
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
        if (collided)
        {
            col.collisions.push_back(closestCollisionInfo);
            col.position = closestCollisionInfo.point;//  + ci.projection;

            remainingVelocity = ci.projection;

			float rh = 0.2f;
			float rw = rh * 2.f;
            const uint8 line_width = 3;
			Renderer::get()->DrawRect(rectf(ci.point.x + ci.projection.x - rh, ci.point.y + ci.projection.y - rh, rw, rw), line_width, DrawLayer_UI, vec4(1, 1, 0, 1));

            // col.position.x = MinSigned(col.position.x, maxX);
            // col.position.y = MinSigned(col.position.y, maxY);
        }
		else if (valid)
        {
            col.position += remainingVelocity;
        }
		else
		{

		}

    }while(iterations++ < maxIterations && collided && length(remainingVelocity) > COLLISION_EPSILON);


    // Just an assertion of curiousity. If this fires, there is nothing wrong, It just means that many
    // bounces are being handled.
    assert(iterations != maxIterations);

    velocity = col.position - startPosition;

    if (length(velocity) < 0.0001f)
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

    PopAllocation(&this->quadtree_memory, collision_list_base);

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
bool CheckCollision(const Rectf& m, Vec2 velocity, Rectf& other, CollisionInfo& out)
{
	bool result = false;
	Vec2 o = { m.x + m.w / 2.f, m.y + m.h / 2.f };

	Ray motion = { { o.x , o.y },
					{ o.x + velocity.x, o.y + velocity.y } };

    // Do the Minkowski sum
    //Vec2 center = rect_center(other); // unused ?
    Rectf mSum = MinkowskiSum(m, other);
#if 0 // moved this to the MinkowskiSum function
    { other.x - m.w / 2.f,
        other.y - m.h / 2.f,
        m.w + other.w,
        m.h + other.h };
#endif

    out.mSumOther = mSum;

#ifdef _DEBUG
	Physics::AddMinkowskiDebugRect(mSum);
#endif

	Ray mRays[4] = {};
	Vec2 p[4] = { { mSum.x, mSum.y },
                  { mSum.x, mSum.y + mSum.h },
                  { mSum.x + mSum.w, mSum.y + mSum.h },
                  { mSum.x + mSum.w, mSum.y} };

#ifdef _DEBUG
	printf(!Contains(mSum, o));
#endif

	mRays[0] = make_ray(p[0], p[1]);
	mRays[1] = make_ray(p[1], p[2]);
	mRays[2] = make_ray(p[2], p[3]);
	mRays[3] = make_ray(p[3], p[0]);

    float closest = -1.f;

	for (int i = 0; i < 4; ++i)
	{
        Vec2 intersection;
        // TODO: SIMD
        if(LineSegmentIntersection(motion.v0, motion.v1, mRays[i].v0, mRays[i].v1, intersection))
        {
            float distance = length(intersection - motion.v0);

            if(!result || distance < closest)
            {
                result = true;
                closest = distance;
                out.distance = distance;

                // Magnitude of projection vector
                float remainingDistanceToTravel = length(velocity) - distance;

                Vec2 b = mRays[i].v1 - mRays[i].v0;
                Vec2 n = Normalize(vec2(-b.y, b.x));
                Vec2 d = motion.v1 - motion.v0;
                Vec2 r = d - (2.f * dot(d, n) * n);

                r = Normalize(r);
                r *= remainingDistanceToTravel;

                out.projection = (dot(r, b) / dot(b, b)) * b;

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
            if(Contains(child, collider->rect))
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
    const uint8 line_width = 2;

    const float blue  = (float)physics_node->depth / 10.f;
    const float green = Contains(physics_node, MouseWorldPosition()) ? 0.8f : 0.1f;

    Vec4 color = { 0, green, blue, 0.5f };
    ren->DrawRect(physics_node->aabb, line_width, DrawLayer_UI, color, LineDrawOptions::CUSTOM_SIZE);

    if(!IsLeaf(physics_node))
    {
        PhysicsNode* node = physics_node->child_nodes;
        for(uint32 i = 0; i < QUADTREE_CHILDREN; ++i, ++node)
        {
            DrawBoundingBoxes(node, ren);
        }
    }

    ren->DrawRect(physics_node->aabb, line_width, DrawLayer_UI, color, LineDrawOptions::CUSTOM_SIZE);
}

