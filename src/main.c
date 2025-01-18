#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <getopt.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include "screen.h"

#define UTIME_SECOND 1000000

typedef long long UTime; // time in microseconds

UTime get_current_utime();
void signal_handler_end(int num);
char get_char_at_point(int x, int y);

// used to control exit
bool g_running = true;

// light level map where as the index increases the character is more dense
const int g_light_level_map_size = 91;
const char g_light_level_map[] = {'`', '.', '-', '\'', ':', '_', ',', '^', '=', ';', '>', '<', '+', '!', 'r', 'c', '*', '/', 'z', '?', 's', 'L', 'T', 'v', ')', 'J', '7', '(', '|', 'F', 'i', '{', 'C', '}', 'f', 'I', '3', '1', 't', 'l', 'u', '[', 'n', 'e', 'o', 'Z', '5', 'Y', 'x', 'j', 'y', 'a', ']', '2', 'E', 'S', 'w', 'q', 'k', 'P', '6', 'h', '9', 'd', '4', 'V', 'p', 'O', 'G', 'b', 'U', 'A', 'K', 'X', 'H', 'm', '8', 'R', 'D', '#', '$', 'B', 'g', '0', 'M', 'N', 'W', 'Q', '%', '&', '@'};

// circle position
int g_circle_x;
int g_circle_y;
int g_circle_z;
int g_circle_radius;

// camera position (at the bottom left corner)
const int g_cam_x = 0;
const int g_cam_y = 0;
const int g_cam_z = 0;

// light starting position
int g_initial_light_x;
int g_initial_light_y;
int g_initial_light_z;

// light position updated by animation
int g_light_x;
int g_light_y;
int g_light_z;

int main(int argc, char **argv)
{
    
    // Option parsing
    int arg_fps = 15;
    int arg_term_width = 20;
    int arg_term_height = 20;
    int arg_circle_radius = 9;
    char arg_light_axis = 'y';
    int arg_light_offset = 20;
    float arg_period = 2.0;

    {

	const struct option long_options[] =
	    {
		{"help", no_argument, 0, 0},
		{"fps", required_argument, 0, 'f'},
		{"term-width", required_argument, 0, 'w'},	
		{"term-height", required_argument, 0, 'h'},
		{"circle-radius", required_argument, 0, 'r'},
		{"light-axis", required_argument, 0, 'a'},
		{"light-offset", required_argument, 0, 'o'},
		{"period", required_argument, 0, 't'},
		{0, 0, 0, 0}
	    };

	int opt_char;
	int option_index;
	
	while ((opt_char = getopt_long(argc, argv, "f:w:h:r:a:o:t:", long_options, &option_index)) != -1)
	{
	    const char *name = long_options[option_index].name;

	    switch(opt_char)
	    {
		case 0:
		{
		    printf("All arguments are optional and are not in any specific order.\nUsage: %s {--help} {--fps=int} {--term-width=int} {--term-height=int} {--circle-radius=int} {--light-axis=[x,y,z]} {--light-offset=int} {--period=float}\n\n\
--help\n\
    Displays the manual\n\
\n\
-f, --fps\n\
    Requires an argument(integer > 0) which specifies the amount of times to render the sphere per second.\n\
    Default: %d\n\
\n\
-w, --term-width\n\
    Requires an argument(integer > 0) which specifies the width of the screen to display the sphere.\n\
    Default: %d\n\
\n\
-h, --term-height\n\
    Requires an argument(integer > 0) which specifies the height of the screen to display the sphere.\n\
    Default: %d\n\
\n\
-r, --circle-radius\n\
    Requires an argument(integer > 0) which specifies the radius of the sphere in characters.\n\
    Default: %d\n\
\n\
-a, --light-axis\n\
    Requires an argument of one of the following characters: x,y,z.\n\
    This argument specifies the axis that the light will rotate around.\n\
    Default: %c\n\
\n\
-o, --light-offset\n\
    Requires an argument(integer) which specifies the offset that the light will have upon it's rotating axis.\n\
    Default: %d\n\
\n\
-t, --period\n\
    Requires an argument(float > 0) which specifies the amount of seconds it takes for the light to make a revolution.\n\
    Default: %f\n\
", argv[0], arg_fps, arg_term_width, arg_term_height, arg_circle_radius, arg_light_axis, arg_light_offset, arg_period);

		    exit(0);
		}

		case 'f':
		{
		    arg_fps = atoi(optarg);

		    if (arg_fps <= 0)
		    {
			printf("ERROR - %s must be a valid integer greater than 0.\n", name);
			exit(0);
		    }
		    break;
		}

		case 'w':
		{
		    arg_term_width = atoi(optarg);

		    if (arg_term_width <= 0)
		    {
			printf("ERROR - %s must be a valid integer greater than 0.\n", name);
			exit(0);
		    }

		    break;
		}

		case 'h':
		{
		    arg_term_height = atoi(optarg);

		    if (arg_term_height <= 0)
		    {
			printf("ERROR - %s must be a valid integer greater than 0.\n", name);
			exit(0);
		    }

		    break;
		}

		case 'r':
		{
		    arg_circle_radius = atoi(optarg);

		    if (arg_circle_radius <= 0)
		    {
			printf("ERROR - %s must be a valid integer greater than 0.\n", name);
			exit(0);
		    }

		    break;
		}

		case 'a':
		{
		    if (strcmp(optarg, "x") != 0 && strcmp(optarg, "y") != 0 && strcmp(optarg, "z") != 0)
		    {
			printf("ERROR - %s must only be one of these values: x, y, z.\n", name);
			exit(0);			
		    }

		    arg_light_axis = optarg[0];

		    break;
		}

		case 'o':
		{
		    arg_light_offset = atoi(optarg);

		    break;
		}

		case 't':
		{
		    arg_period = atof(optarg);

		    if (arg_period <= 0)
		    {
			printf("ERROR - Failed to parse %s arg, make sure it is a valid real number greater than 0.\n", name);
			exit(0);
		    }

		    break;
		}
	    }
	}
    }

    // Validate argument interdependencies
    {
	int min = (arg_term_width < arg_term_height)? arg_term_width : arg_term_height;
	if (min < arg_circle_radius * 2 + 1)
	{
	    printf("ERROR - Your circle is too big for your terminal.\nTo fix this either make sure your terminal is at least %dx%d, or decrease the size of your circle radius to at greatest %d\n", (arg_circle_radius*2 + 1), (arg_circle_radius*2 + 1), min/2 - 1);
	    exit(0);
	}

    }

    // Use the args to calculate other values
    {
	// circle position
	g_circle_radius = arg_circle_radius;
	g_circle_x = arg_term_width / 2;
	g_circle_y = arg_term_height / 2;
	g_circle_z = g_circle_radius + 1;
	
	// light starting position
	g_initial_light_x = g_circle_x;
	g_initial_light_y = g_circle_y;
	g_initial_light_z = g_circle_z;
	switch (arg_light_axis)
	{
	    case 'x':
		g_initial_light_x += arg_light_offset;
		break;

	    case 'y':
		g_initial_light_y += arg_light_offset;
		break;

	    case 'z':
		g_initial_light_z += arg_light_offset;
		break;
	}
	
	// light position updated by animation
	g_light_x = g_initial_light_x;
	g_light_y = g_initial_light_y;
	g_light_z = g_initial_light_z;
    }

    
    Screen screen = screen_create(arg_term_width, arg_term_height);

    // Time vars
    UTime tick_length = UTIME_SECOND / arg_fps;
    UTime new_time;
    UTime old_time = get_current_utime() - tick_length;

    // Animation vars
    double anim_circle_period = arg_period * UTIME_SECOND;
    double anim_circle_radius = g_circle_radius * 2;
    UTime anim_start = old_time;

    // make interupts gracefully exit
    signal(SIGINT, signal_handler_end);

    while (g_running)
    {
	screen_empty(&screen);

	// Timer
	{
	    new_time = get_current_utime();
	    while (old_time + tick_length > new_time)
	    {
		usleep(old_time + tick_length - new_time);
		new_time = get_current_utime();
	    }
	    old_time += tick_length;
	}

	// Change position of light based on animation
	double theta = M_PI * 2.0 / anim_circle_period * (old_time - anim_start);
	switch (arg_light_axis)
	{
	    case 'x':
	    {
		g_light_y = g_initial_light_y + anim_circle_radius * cos(theta);
		g_light_z = g_initial_light_z + anim_circle_radius * sin(theta);
		break;
	    }

	    case 'y':
	    {
		g_light_x = g_initial_light_x + anim_circle_radius * cos(theta);
		g_light_z = g_initial_light_z + anim_circle_radius * sin(theta);
		break;
	    }

	    case 'z':
	    {
		g_light_x = g_initial_light_x + anim_circle_radius * cos(theta);
		g_light_y = g_initial_light_y + anim_circle_radius * sin(theta);
		break;
	    }
	}

	// get the character for each position in the screen and write it to the screen
	for (int y = 0; y < screen.height; ++y)
	{
	    for (int x = 0; x < screen.width; ++x)
	    {
		screen_write(&screen, x, y, get_char_at_point(x, y));
	    }
	}

	// display the screen
	screen_print(&screen);

    }

    // cleanup terminal
    screen_empty(&screen);
    screen_print(&screen);

    // cleanup resources
    screen_destroy(&screen);

    return 0;
}

char get_char_at_point(int screen_x, int screen_y)
{
    int dx = g_circle_x - screen_x;
    int dy = g_circle_y - screen_y;

    // use pythagorean theorem to tell if point is in the circle (sqrt(x^2 + y^2) <= r then x^2 + y^2 <= r^2)
    if ((dx * dx + dy * dy) <= (g_circle_radius * g_circle_radius))
    {
	double divisor; // used as the magnitude to divide vectors by to normalize them

	// get the point based on screen position
	int x = screen_x;
	int y = screen_y;
	int z = g_circle_z - (int)(sqrt((double)g_circle_radius * g_circle_radius - ((double)dx * dx + (double)dy * dy))); // base - sqrt(r^2 - x^2) to find z value

	// normal vector
	double norm_x = x - g_circle_x;
	double norm_y = y - g_circle_y;
	double norm_z = z - g_circle_z;

	divisor = sqrt(norm_x*norm_x + norm_y*norm_y + norm_z*norm_z);
	norm_x /= divisor;
	norm_y /= divisor;
	norm_z /= divisor;

	// light_to_point vector
	double light_to_point_x = x - g_light_x;
	double light_to_point_y = y - g_light_y;
	double light_to_point_z = z - g_light_z;

	divisor = sqrt(light_to_point_x*light_to_point_x + light_to_point_y*light_to_point_y + light_to_point_z*light_to_point_z);
	light_to_point_x /= divisor;
	light_to_point_y /= divisor;
	light_to_point_z /= divisor;

	// reflect light_to_point vector over normal vector
	double k = 2.0*(light_to_point_x* norm_x + light_to_point_y*norm_y + light_to_point_z*norm_z);
	double reflect_x = light_to_point_x - k * norm_x;
	double reflect_y = light_to_point_y - k * norm_y;
	double reflect_z = light_to_point_z - k * norm_z;

/* unecessary to normalize the reflect vector
	divisor = sqrt(reflect_x*reflect_x + reflect_y*reflect_y + reflect_z*reflect_z);
	reflect_x /= divisor;
	reflect_y /= divisor;
	reflect_z /= divisor;
*/

	// point_to_cam vector
	double point_to_cam_x = (g_cam_x + screen_x) - x; // adds screen_x because it is orthographic projection
	double point_to_cam_y = (g_cam_y + screen_y) - y;
	double point_to_cam_z = g_cam_z - z;

	divisor = sqrt(point_to_cam_x*point_to_cam_x + point_to_cam_y*point_to_cam_y + point_to_cam_z*point_to_cam_z);
	point_to_cam_x /= divisor;
	point_to_cam_y /= divisor;
	point_to_cam_z /= divisor;

	// take dot product of point_to_cam and reflect vector (sees how similar the direction of the reflected light is to the direction towards the camera)
	double dot_product = (point_to_cam_x * reflect_x + point_to_cam_y * reflect_y + point_to_cam_z * reflect_z);

	// if dot_product < 0 then the light at that point is obstructed by the sphere
	if (dot_product < 0)
	{
	    return g_light_level_map[0];
	}

	return g_light_level_map[(int)(dot_product * (g_light_level_map_size - 1))];
    }
    else
    {
	return ' ';
    }
}

UTime get_current_utime()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) == -1)
    {
	perror("gettimeofday");
	exit(1);
    }

    return tv.tv_sec * UTIME_SECOND + tv.tv_usec;
}

void signal_handler_end(int num)
{
    g_running = false;
}
