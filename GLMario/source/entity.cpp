#include "entity.h"

void entity_update(Entity& e, float delta_time)
{
	static Input* input = Input::get();
	static Renderer* ren = Renderer::get();
	EntityType::Type t = e.type;
    switch(t)
    {
        case EntityType::PLAYER:
        {
            static uint32 count = 0;
            if(input->is_down(SDLK_w))
            {
				e.pe.velocity.y = e.pe.max_hor_vel;
            }   
            else if(input->is_down(SDLK_s))
            {
				// NOTE USING HOR VEL FOR NOW UNTIL JUMPING IS IN
				e.pe.velocity.y = -e.pe.max_hor_vel;
            }
            else
            {
                e.pe.velocity.y = 0;
            }

            if(input->is_down(SDLK_d))
            {
				e.pe.velocity.x = e.pe.max_hor_vel;
            }   
            else if(input->is_down(SDLK_a))
            {
				e.pe.velocity.x = -e.pe.max_hor_vel;
            }
            else
            {
                e.pe.velocity.x = 0;
            }
			e.position += e.pe.velocity * delta_time;

            std::string p_info("New Player x: " + std::to_string(e.position.x) + "\nNew Player y: " + std::to_string(e.position.y));
            Console::get()->log_message(p_info);

            //sprite.color_mod.w = 0.
			e.pe.draw_call.sd.world_position = e.position;
            ren->push_draw_call(e.pe.draw_call, DrawLayer::PLAYER);
                // ren->draw_sprite(&sprite, transform.position);
        } break;
        case EntityType::TILE:
        {
            ren->push_draw_call(e.te.draw_call, DrawLayer::TILEMAP);
        }break;
		case EntityType::PARTICLE_SYSTEM:
        {
            if(e.pse.ps)
            {
                e.pse.ps->update(e.position);
                e.pse.ps->render();
            }
        }break;
        default:
        {
            assert(0);
        }break;
    }
}

void draw_entity(EntityDrawer ed)
{

}

void simulate_physics(EntityPhysics ep)
{

}
