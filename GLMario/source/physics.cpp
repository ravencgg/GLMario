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

void SetPosition(RDynamicCollider col, Vec2 position)
{
    TDynamicCollider* c = col.data;
    c->position = position;
}

Vec2 GetPosition(RDynamicCollider col)
{
    Vec2 result = col.data->position;
    return result;
}

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

    for(uint32 i = 0; i < active_dynamics.size(); ++i)
    {
        TDynamicCollider& col = dynamics[active_dynamics[i]];
        col.collisions.resize(0);

        for(uint32 j = 0; j < active_statics.size(); ++j)
        {
            TStaticCollider& scol = statics[active_statics[j]];

            CollisionInfo ci;
			if (CheckCollision(col.rect, col.velocity, scol.rect, ci))
            {
                col.collisions.push_back(ci);
            }
        }
    }
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

        std::vector<SimpleVertex> verts;
        SimpleVertex v = {};
        v.position = dynamics[loc].position;
        v.color    = vec4(1, 0, 0, 1);
        verts.push_back(v);
        v.position = dynamics[loc].position + dynamics[loc].velocity;
        v.color    = vec4(1, 1, 0, 1);
        verts.push_back(v);

        ren->DrawLine(verts, DrawLayer::UI);
    }
}


// TODO(cgenova): move into the Physics class and call during Step()
bool CheckCollision(Rectf& m, Vec2& velocity, Rectf& other, CollisionInfo& out)
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
