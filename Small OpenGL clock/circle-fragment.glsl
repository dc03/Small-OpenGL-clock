#version 330 core

in vec3 frag_pos;

uniform vec3 circle_color;
uniform float radius;
uniform float line_length;

out vec4 frag_result;

void main()
{
    float S1 = dot(frag_pos, frag_pos) - (radius * radius);
    float dist_origin = dot(frag_pos, frag_pos);

    // I hardcoded the values of tan(30), tan(60), -tan(30) and -tan(60) to save
    // cycles on the GPU

    // AA like thing
    if ((0.008f < S1) && (S1 < 0.02f))
    {
        frag_result = vec4(circle_color * 0.6f, 0.3f);
        return;
    }
    // Circle shading within tolerance
    else if ((-0.008f < S1) && (S1 <= 0.008f))
    {
        frag_result = vec4(circle_color, 1.0f);
        return;
    }
    // AA like thing
    else if ((-0.02f < S1) && (S1 <= -0.008f))
    {
        frag_result = vec4(circle_color * 0.6f, 0.3f);
    }
    // Checking if the fragment is far enough away from the origin to form line
    // but it also needs to be constrained inside the circle
    else if (((line_length * line_length) < dist_origin) &&
              (dist_origin < (radius * radius)))
    {
        float slope = -1.0f;
        if (frag_pos.x != 0)
            slope = frag_pos.y / frag_pos.x;

        // Numbers 12, 3, 6, 9
        if ((-0.005f < frag_pos.y && frag_pos.y < 0.005f) ||
            (-0.005f < frag_pos.x && frag_pos.x < 0.005f))
            frag_result = vec4(circle_color, 1.0f);
        else if ((-0.010f < frag_pos.y && frag_pos.y < 0.010f) ||
                 (-0.010f < frag_pos.x && frag_pos.x < 0.010f))
            frag_result = vec4(circle_color * 0.6f, 0.3f);
        else if (slope != -1.0f)
        {
            // Number 1 and Number 6
            if (1.707f < slope && slope < 1.757f)
                frag_result = vec4(circle_color, 1.0f);
            // AA like thing
            else if (1.690f < slope && slope < 1.772f)
                frag_result = vec4(circle_color * 0.6f, 0.3f);  

            // Number 2 and Number 7
            else if (0.562f < slope && slope < 0.578f)
                frag_result = vec4(circle_color, 1.0f);
            // AA like thing
            else if (0.557f < slope && slope < 0.583f)
                frag_result = vec4(circle_color * 0.6f, 0.3f);
            
            // Number 5 and Number 11
            else if (-1.757f < slope && slope < -1.707f)
                frag_result = vec4(circle_color, 1.0f);
            // AA like thing
            else if (-1.772f < slope && slope < -1.690f)
                frag_result = vec4(circle_color * 0.6f, 0.3f);

            // Number 4 and Number 10
            else if (-0.578f < slope && slope < -0.562f)
                frag_result = vec4(circle_color, 1.0f);
            // AA like thing
            else if (-0.583f < slope && slope < -0.557f)
                frag_result = vec4(circle_color * 0.6f, 0.3f);
            else
                discard;
        }
        else
            discard;
    }
    else
        discard;
}