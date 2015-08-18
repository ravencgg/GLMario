#include "physics.h"


Physics::Physics()
{
	statics = new TStaticCollider[Max_Static_Colliders];
    dynamics = new TDynamicCollider[Max_Dynamic_Colliders];

    active_statics.reserve(Max_Static_Colliders);
    active_dynamics.reserve(Max_Static_Colliders);

    inactive_statics.reserve(Max_Static_Colliders);
    inactive_dynamics.reserve(Max_Static_Colliders);

    for(int i = Max_Static_Colliders - 1; i >= 0; --i) 
    {
        inactive_dynamics.push_back(i);
    }
    for(int i = Max_Dynamic_Colliders - 1; i >= 0; --i) 
    {
        inactive_statics.push_back(i);
    }
}

Physics::~Physics()
{
    if(statics) delete[] statics;
    if(dynamics) delete[] dynamics;
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
    assert(col.array_index < Max_Dynamic_Colliders);
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

void Physics::Step(float dt)
{
	// NOTE(cgenova): Bubble sort would be faster here.
	std::sort(active_dynamics.begin(), active_dynamics.end());

	assert(!"Not physics stepping!");
}

// TODO(cgenova): move into the Physics class and call during Step()
bool check_collision(PhysicsRect& m, Vec2& velocity, PhysicsRect& other, CollisionData& out)
{
	bool result = false;
	Vec2 o = { m.col_rect.x + m.col_rect.w / 2.f, m.col_rect.y + m.col_rect.h / 2.f };

	Ray motion = { { o.x , o.y }, 
					{ o.x + velocity.x, o.y + velocity.y } };
	
	// Do the Minkowski sum
	Vec2 center = rect_center(other.col_rect);
	Rectf mSum = { other.col_rect.x - m.col_rect.w / 2.f,
					other.col_rect.y - m.col_rect.h / 2.f,
					m.col_rect.w + other.col_rect.w,
					m.col_rect.h + other.col_rect.h };

	Ray mRays[4] = {};
	Vec2 p[4] = { { mSum.x, mSum.y },
						{ mSum.x, mSum.y + mSum.h },
						{ mSum.x + mSum.w, mSum.y + mSum.h },
						{ mSum.x + mSum.w, mSum.y} };

	mRays[0] = make_ray(p[0], p[1]);
	mRays[1] = make_ray(p[1], p[2]);
	mRays[2] = make_ray(p[2], p[3]);
	mRays[3] = make_ray(p[3], p[0]);

	for (int i = 0; i < 4; ++i)
	{
		Vec2 e = motion.v1 - motion.v0;
		Vec2 f = mRays[i].v1 - mRays[i].v0;

		Vec2 p = { -e.y, e.x };

		float dfp = dot(f, p);

		if (dfp > 0)
		{
			float h = (dot(motion.v0 - mRays[i].v0, p) / dot(f, p));

			Vec2 impact = mRays[i].v0 + f * h;

			// TODO(cgenova): clean up this loop
			if (h > 0 && h < 1 && !result)
			{
				result = true;
				out.distance = length(impact - o);
				out.point = impact;
			}
			else if (h > 0 && h < 1)
			{
				float l = length(impact - o);
				if (out.distance > l)
				{
					out.distance = l;
					out.point = impact;
				}
			}
		}
	}
	return result;
}
