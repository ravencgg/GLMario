#version 330 core
in vec4 color_modifier;
in vec2 frag_tex_coord;
out vec4 color;
uniform sampler2D ourTexture;

void main()
{
	color = texture(ourTexture, frag_tex_coord);// + vec4(1.0f, 1.0f, 0f, 0f);// * ((sin(time) + 1) / 2
        // TODO(cgenova): color modification;
    // color *= color_modifier;

//    color.a *= color_modifier.a;

//TODO(chris): Modify out color by input color -> this line is untested

	// color = new_position;//  = theColor * ((sin(time) + 1) / 2);
	//outputColor = vec4(0.0f, 0.75f, 0.5f, 1.0f);
}

/*
const float pixelSize  =    4.00; // base pixelization 
const float zoom       =   64.00; // zoom range
const float radius     =  128.00; // planar movement radius
const float speed      =    0.50; // zoom / move speed

////////////////////////////////////////////////////////////////////////////////////////////////////

float time   = iGlobalTime * speed;
float scale  = pixelSize + ((sin(time / 3.7) + 1.0) / 2.0) * (zoom - 1.0) * pixelSize;

vec2  center = iResolution.xy / 2.0;
vec2  offset = vec2(cos(time), sin(time)) * radius;

////////////////////////////////////////////////////////////////////////////////////////////////////

void mainImage( out vec4 color, in vec2 pixel )
{
    // add some movement:
    pixel = ((pixel + offset) - center) / scale + center;

    // basic sampling:
    vec2 uv = floor(pixel) + 0.5;
    
    // subpixel filtering algorithm (comment out to compare):
    uv += 1.0 - clamp((1.0 - fract(pixel)) * scale, 0.0, 1.0);

   	color = texture2D(iChannel0, uv  / iChannelResolution[0].xy);
}
*/