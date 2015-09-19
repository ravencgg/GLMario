#include "physics.h"

Rectf CanonicalRect(TDynamicCollider* col)
{
    Rectf result = {};
    result.x = col->position.x + col->rect.x;
    result.y = col->position.y + col->rect.y;
    result.w = col->rect.w;
    result.h = col->rect.h;

    return result;
}

std::vector<Rectf> Physics::minkowski_rects;

void ColliderSetPosition(RDynamicCollider col, Vec2 position)
{
    TDynamicCollider* c = col.data;
    c->position = position;
}

Vec2 ColliderGetPosition(RDynamicCollider col)
{
    Vec2 result = col.data->position;
    return result;
}

Physics::Physics()
{
	statics = new TStaticCollider[MAX_STATIC_COLLIDERS];
    dynamics = new TDynamicCollider[MAX_DYNAMIC_COLLIDERS];

    active_statics.reserve(MAX_STATIC_COLLIDERS);
    active_dynamics.reserve(MAX_DYNAMIC_COLLIDERS);

    inactive_statics.reserve(MAX_STATIC_COLLIDERS);
    inactive_dynamics.reserve(MAX_DYNAMIC_COLLIDERS);

    for(int i = MAX_STATIC_COLLIDERS - 1; i >= 0; --i) 
    {
        inactive_dynamics.push_back(i);
    }
    for(int i = MAX_DYNAMIC_COLLIDERS - 1; i >= 0; --i) 
    {
        inactive_statics.push_back(i);
    }
}

Physics::~Physics()
{
    if(statics) 
    {
        delete[] statics;
        statics = nullptr;
    }
    if(dynamics) 
    {
        delete[] dynamics;
        dynamics = nullptr;
    }
}

RStaticCollider Physics::AddStaticCollider(Rectf r)
{
    TStaticCollider col;
    col.active = true;
    col.rect = r;

    return AddStaticCollider(col);
}

RStaticCollider Physics::AddStaticCollider(TStaticCollider col)
{
    assert(inactive_statics.size() > 0);

    RStaticCollider result;
    uint32 location = inactive_statics.back();
    inactive_statics.pop_back();
    active_statics.push_back(location);

    statics[location] = col;

    result.data = &statics[location];
    result.array_index = location;

    return result;
}

RDynamicCollider Physics::AddDynamicCollider(TDynamicCollider col)
{
    assert(inactive_dynamics.size() > 0);
    RDynamicCollider result = {};

    uint32 location = inactive_dynamics.back();
	inactive_dynamics.pop_back();
    active_dynamics.push_back(location);

    dynamics[location] = col;

    result.data = &dynamics[location];
    result.array_index = location;
	return result;
}

void Physics::RemoveDynamicCollider(RDynamicCollider col)
{
#ifdef _DEBUG
    assert(col.array_index < MAX_DYNAMIC_COLLIDERS);
    memset(dynamics[col.array_index], 0xBD, sizeof(TStaticCollider));
#endif

    bool found = false;
    uint32 index = 0;

    for(uint32 i = 0; i < active_dynamics.size(); ++i)
    {
        if(col.array_index == active_dynamics[i])
        {
            found = true;
            index = i;
            break; 
        }
    }

    if(found)
    {
        inactive_dynamics.push_back(index);
		auto it = active_dynamics.front() + index;
        std::swap(it, active_dynamics.back());
        active_dynamics.pop_back();
    }

}

void Physics::AddMinkowskiDebugRect(Rectf rect)
{
	Physics::minkowski_rects.push_back(rect);
}

void Physics::DebugDraw()
{
    // call this after "step" so that the arrays are already sorted 
    Renderer* ren = Renderer::get();
    DrawLayer dl = DrawLayer::UI;

    Vec4 s_active = vec4(1, 0, 0, 1);
    Vec4 s_inactive = s_active;
    s_inactive.a = 0.5f;
    Vec4 d_active = vec4(0.3f, 0.3f, 1, 1);
    Vec4 d_inactive = d_active;
    d_inactive.a = 0.5f;

    for(uint32 i = 0; i < active_statics.size(); ++i)
    {
        uint32 loc = active_statics[i];
        ren->DrawRect(statics[loc].rect, dl, statics[loc].active ? s_active : s_inactive);
    }

    for(uint32 i = 0; i < active_dynamics.size(); ++i)
    {
        uint32 loc = active_dynamics[i];
        ren->DrawRect(CanonicalRect(&dynamics[loc]), dl, dynamics[loc].active ? d_active : d_inactive);
    }

#ifdef DEBUG
	for (uint32 i = 0; i < Physics::minkowski_rects.size(); ++i)
    {
		ren->DrawRect(Physics::minkowski_rects[i], dl, { 0, 1, 0, 1 });
    }
	Physics::minkowski_rects.clear();
#endif
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

    for(uint32 i = 0; i < active_statics.size(); ++i)
    {
        TStaticCollider& scol = statics[active_statics[i]];

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

// Move to mathops.h
float MaxSigned(float in, float maxValue)
{
	float result = sign(in) * max(abs(in), abs(maxValue));
	return result;
}

float MinSigned(float in, float minValue)
{
	float result = sign(in) * min(abs(in), abs(minValue));
	return result;
}

bool Contains(Rectf rect, Vec2 point)
{
	if (point.x < rect.x + rect.w && point.x > rect.x
		&& point.y > rect.y && point.y < rect.y + rect.h)
	{
		return true;
	}
	return false;
}

Vec2 Physics::StepCollider(RDynamicCollider refCollider, Vec2& velocity, float dt)
{
    TDynamicCollider& col = dynamics[refCollider.array_index];
    col.collisions.resize(0);

    Vec2 startPosition = col.position;

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

    do
    {
        collided = false;
        memset(&ci, 0, sizeof(CollisionInfo));

        for(uint32 j = 0; j < active_statics.size(); ++j)
        {
            TStaticCollider& scol = statics[active_statics[j]];
            if (!scol.active) continue;

			if (CheckCollision(CanonicalRect(&col), remainingVelocity, scol.rect, ci))
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
                for(uint32 k = 0; k < active_statics.size(); ++k)
                {
                    if(!statics[active_statics[k]].active) continue;
                    
					Rectf& other = statics[active_statics[k]].rect;

                    Rectf mSum = { other.x - curRect.w / 2.f,
                            other.y - curRect.h / 2.f,
                            curRect.w + other.w,
                            curRect.h + other.h };

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
        }
        if (collided)
        {
            col.collisions.push_back(closestCollisionInfo);
            col.position = closestCollisionInfo.point;//  + ci.projection;

            remainingVelocity = ci.projection;

			float rh = 0.2f;
			float rw = rh * 2.f;
			Renderer::get()->DrawRect(rectf(ci.point.x + ci.projection.x - rh, ci.point.y + ci.projection.y - rh, rw, rw), DrawLayer::UI, vec4(1, 1, 0, 1));

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
    
    velocity = col.position - startPosition;

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
bool CheckCollision(Rectf& m, Vec2 velocity, Rectf& other, CollisionInfo& out)
{
	bool result = false;
	Vec2 o = { m.x + m.w / 2.f, m.y + m.h / 2.f };

	Ray motion = { { o.x , o.y }, 
					{ o.x + velocity.x, o.y + velocity.y } };
	
	// Do the Minkowski sum
	Vec2 center = rect_center(other);
	Rectf mSum = { other.x - m.w / 2.f,
					other.y - m.h / 2.f,
					m.w + other.w,
					m.h + other.h };

    out.mSumOther = mSum;

#ifdef DEBUG
	Physics::AddMinkowskiDebugRect(mSum);
#endif

	Ray mRays[4] = {};
	Vec2 p[4] = { { mSum.x, mSum.y },
                  { mSum.x, mSum.y + mSum.h },
                  { mSum.x + mSum.w, mSum.y + mSum.h },
                  { mSum.x + mSum.w, mSum.y} };

//	assert(!Contains(mSum, o));

	mRays[0] = make_ray(p[0], p[1]);
	mRays[1] = make_ray(p[1], p[2]);
	mRays[2] = make_ray(p[2], p[3]);
	mRays[3] = make_ray(p[3], p[0]);

    float closest = -1.f;

	for (int i = 0; i < 4; ++i)
	{
        Vec2 intersection;
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
                // out.projection = r;

                // out.projection *= remainingDistanceToTravel;

                // out.projection.x = abs(out.projection.x) * sign(velocity.x);
                // out.projection.y = abs(out.projection.y) * sign(velocity.y);
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
