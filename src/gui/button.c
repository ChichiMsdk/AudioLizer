#include "app.h"

int					g_playing;

Mouse_state
get_mouse_state(void)
{
	float x, y;
	Mouse_state mouse;
	uint32_t flags = SDL_GetMouseState(&x, &y);
	mouse.pos = vec2f(x, y);
	mouse.flags = flags;
	return mouse;
}

/* data struct to loop through all the buttons */
void
loop_gui(Button **buttons)
{

}

void
button_check_hover(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	int i = 0;
	/* checks if inside button */
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			button[i].hovered = true;
			/* create array of cursor beforehand */
			SDL_SetCursor(g_inst.cursorclick);
			break;
		}
		else
		{
			button[i].hovered = false;
			/* button[i].pressed = false; */
			SDL_SetCursor(g_inst.cursordefault);
		}
		i++;
	}
}

/* note: make a function pointer callback for the button when pressed */
void
button_check_pressed(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);

	int i = 0;
	/* checks if inside button */
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			/* create array of cursor beforehand */
			if (m_button == SDL_BUTTON_LEFT)
			{
				button[i].pressed = true;
			}
		}
		else
		{
			button[i].pressed = false;
		}
		i++;
	}
}

void
button_check_released(Mouse_state mouse, Button *button)
{
	Vec2f m_pos = screen_to_world(g_inst.cam, (Vec2f){mouse.pos.x, mouse.pos.y});
	uint32_t m_button = SDL_BUTTON(mouse.flags);
	/* checks if inside button */
	int i = 0;
	while (i < button->count)
	{
		if ((m_pos.x < button[i].rect.w + button[i].rect.x
				&& m_pos.x >= button[i].rect.x)
				&& (m_pos.y < button[i].rect.h + button[i].rect.y
				&& m_pos.y >= button[i].rect.y)) 
		{
			if (button[i].pressed)
			{
				{
				/* create array of cursor beforehand */
					button[i].released = true;
					if (button[i].fn)
						button[i].fn(&g_play_sfx);
				}
			}
		}
		button[i].pressed = false;
		i++;
	}
}

void*
test(void *i)
{
	printf("Test\n");
	return NULL;
}

/* giant sprite instead ? */
char const svg[] = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" height=\"32\" viewBox=\"0 0 48 48\"><path fill=\"currentColor\" d=\"M25.081 6.419C26.208 5.408 28 6.207 28 7.72v32.56c0 1.514-1.792 2.313-2.919 1.302l-8.206-7.366A4.75 4.75 0 0 0 13.702 33H9a5.25 5.25 0 0 1-5.25-5.25v-7.5C3.75 17.35 6.1 15 9 15h4.702a4.75 4.75 0 0 0 3.173-1.216zm.419 2.983l-6.955 6.244a7.25 7.25 0 0 1-4.843 1.854H9a2.75 2.75 0 0 0-2.75 2.75v7.5A2.75 2.75 0 0 0 9 30.5h4.702a7.25 7.25 0 0 1 4.843 1.855L25.5 38.6zm10.838-1.006a1.25 1.25 0 0 1 1.766-.058h.001l.01.01l.019.018a6 6 0 0 1 .262.267c.17.18.404.441.682.783a20.4 20.4 0 0 1 2.016 3.005C42.553 15.059 44 18.953 44 24s-1.447 8.94-2.906 11.58a20.4 20.4 0 0 1-2.016 3.004a15 15 0 0 1-.885.992l-.06.058l-.018.018l-.006.006l-.003.002v.001l.22-.22c.107-.107.143-.144-.222.22a1.25 1.25 0 0 1-1.71-1.822v-.001l.004-.003a5 5 0 0 0 .179-.183c.131-.14.326-.355.563-.647a18 18 0 0 0 1.766-2.635C40.198 32.034 41.5 28.553 41.5 24s-1.302-8.034-2.594-10.37a18 18 0 0 0-1.766-2.636a12 12 0 0 0-.71-.798l-.032-.032l-.003-.003l-.002-.002a1.25 1.25 0 0 1-.055-1.764M32.334 14.4a1.25 1.25 0 0 1 1.767-.065l.001.001l.002.002l.005.005l.014.012l.042.041q.051.05.137.139c.113.118.269.287.452.505c.366.436.847 1.072 1.326 1.893A14 14 0 0 1 38 24c0 3.023-.963 5.426-1.92 7.068c-.48.82-.96 1.457-1.326 1.893a10 10 0 0 1-.59.644l-.019.019l-.022.021l-.014.013l-.005.005l-.002.003H34.1a1.25 1.25 0 0 1-1.705-1.828l.002-.002l.016-.016l.085-.086c.078-.081.196-.209.34-.381c.29-.346.685-.866 1.081-1.545A11.5 11.5 0 0 0 35.5 24c0-2.477-.787-4.449-1.58-5.807c-.396-.68-.79-1.2-1.08-1.545a8 8 0 0 0-.426-.467l-.017-.017l.001.001a1.25 1.25 0 0 1-.064-1.765m5.781-6.052l-.01-.01l.001.002l.003.002zm-4.014 5.986l.064.062z\"/></svg>";
char const fscreen_filled[] = "<svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M4 3.5C3.72386 3.5 3.5 3.72386 3.5 4V5.61364C3.5 6.02785 3.16421 6.36364 2.75 6.36364C2.33579 6.36364 2 6.02785 2 5.61364V4C2 2.89543 2.89543 2 4 2H5.61364C6.02785 2 6.36364 2.33579 6.36364 2.75C6.36364 3.16421 6.02785 3.5 5.61364 3.5H4ZM9.63636 2.75C9.63636 2.33579 9.97215 2 10.3864 2H12C13.1046 2 14 2.89543 14 4V5.61364C14 6.02785 13.6642 6.36364 13.25 6.36364C12.8358 6.36364 12.5 6.02785 12.5 5.61364V4C12.5 3.72386 12.2761 3.5 12 3.5H10.3864C9.97215 3.5 9.63636 3.16421 9.63636 2.75ZM2.75 9.63636C3.16421 9.63636 3.5 9.97215 3.5 10.3864V12C3.5 12.2761 3.72386 12.5 4 12.5H5.61364C6.02785 12.5 6.36364 12.8358 6.36364 13.25C6.36364 13.6642 6.02785 14 5.61364 14H4C2.89543 14 2 13.1046 2 12V10.3864C2 9.97215 2.33579 9.63636 2.75 9.63636ZM13.25 9.63636C13.6642 9.63636 14 9.97215 14 10.3864V12C14 13.1046 13.1046 14 12 14H10.3864C9.97215 14 9.63636 13.6642 9.63636 13.25C9.63636 12.8358 9.97215 12.5 10.3864 12.5H12C12.2761 12.5 12.5 12.2761 12.5 12V10.3864C12.5 9.97215 12.8358 9.63636 13.25 9.63636Z\" fill=\"#ffffff\"/> </svg>";
char const rscreen_filled[] = "<svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M3 6.25C3 4.45507 4.45507 3 6.25 3H10.5C11.0523 3 11.5 3.44772 11.5 4C11.5 4.55228 11.0523 5 10.5 5H6.25C5.55964 5 5 5.55964 5 6.25V10.5C5 11.0523 4.55228 11.5 4 11.5C3.44772 11.5 3 11.0523 3 10.5V6.25ZM20.5 4C20.5 3.44772 20.9477 3 21.5 3H25.75C27.5449 3 29 4.45507 29 6.25V10.5C29 11.0523 28.5523 11.5 28 11.5C27.4477 11.5 27 11.0523 27 10.5V6.25C27 5.55964 26.4404 5 25.75 5H21.5C20.9477 5 20.5 4.55228 20.5 4ZM4 20.5C4.55228 20.5 5 20.9477 5 21.5V25.75C5 26.4404 5.55964 27 6.25 27H10.5C11.0523 27 11.5 27.4477 11.5 28C11.5 28.5523 11.0523 29 10.5 29H6.25C4.45507 29 3 27.5449 3 25.75V21.5C3 20.9477 3.44772 20.5 4 20.5ZM28 20.5C28.5523 20.5 29 20.9477 29 21.5V25.75C29 27.5449 27.5449 29 25.75 29H21.5C20.9477 29 20.5 28.5523 20.5 28C20.5 27.4477 20.9477 27 21.5 27H25.75C26.4404 27 27 26.4404 27 25.75V21.5C27 20.9477 27.4477 20.5 28 20.5Z\" fill=\"#ffffff\"/> </svg>";
char const cursor[] = "<svg fill=\"none\" height=\"20\" viewBox=\"0 0 20 20\" width=\"20\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"m6.63564 2.28741c-.65239-.53716-1.63564-.07309-1.63564.77199v13.9988c0 .9261 1.15025 1.3547 1.75622.6543l3.52368-4.0724c.2849-.3293.6989-.5185 1.1343-.5185h5.5919c.9383 0 1.36-1.1756.6357-1.772z\" fill=\"#212121\"/></svg>";
char const cursorclick[] = "<svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M7.99945 0.997559C6.89379 0.997559 5.99747 1.89387 5.99747 2.99954V7.20445L5.37103 6.92674C3.90134 6.27521 2.17688 6.70601 1.18625 7.97216C0.827729 8.4304 1.01093 9.09236 1.52937 9.31944C3.52984 10.1957 4.73506 10.9235 5.5142 11.5868C6.27879 12.2377 6.64842 12.8391 6.95406 13.5047C7.41036 14.4983 8.43659 15.1251 9.5537 14.9743L11.6242 14.6949C12.6392 14.558 13.4679 13.8158 13.7157 12.8221L14.397 10.089C14.8948 8.09197 13.5703 6.09852 11.5364 5.78365L10.0014 5.54603V2.99953C10.0014 1.89387 9.10511 0.997559 7.99945 0.997559Z\" fill=\"#212121\"/> </svg>";
char const svgplay[] = "<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M5 5.27368C5 3.56682 6.82609 2.48151 8.32538 3.2973L20.687 10.0235C22.2531 10.8756 22.2531 13.124 20.687 13.9762L8.32538 20.7024C6.82609 21.5181 5 20.4328 5 18.726V5.27368Z\" fill=\"#ffffff\"/> </svg> ";
char const svgstop[] = "<svg width=\"16\" height=\"16\" viewBox=\"0 0 16 16\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M3.5 2C2.67157 2 2 2.67157 2 3.5V12.5C2 13.3284 2.67157 14 3.5 14H12.5C13.3284 14 14 13.3284 14 12.5V3.5C14 2.67157 13.3284 2 12.5 2H3.5Z\" fill=\"#ffffff\"/> </svg>";
char const svgreplay[] = "<svg width=\"32\" height=\"32\" viewBox=\"0 0 32 32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\"> <path d=\"M7.05557 8C9.25286 5.545 12.446 4 16 4C22.6274 4 28 9.37258 28 16C28 22.6274 22.6274 28 16 28C9.37258 28 4 22.6274 4 16C4 15.7173 4.00978 15.4368 4.02902 15.1589C4.07121 14.5495 3.60935 14 2.99844 14C2.50687 14 2.08625 14.3612 2.04646 14.8512C2.01569 15.2301 2 15.6132 2 16C2 23.732 8.26801 30 16 30C23.732 30 30 23.732 30 16C30 8.26801 23.732 2 16 2C12.0826 2 8.54103 3.60893 6 6.20203V4C6 3.44772 5.55228 3 5 3C4.44772 3 4 3.44772 4 4V9C4 9.55228 4.44772 10 5 10H10C10.5523 10 11 9.55228 11 9C11 8.44772 10.5523 8 10 8H7.05557ZM14.9921 10.266C13.6588 9.5043 12 10.467 12 12.0026V19.9977C12 21.5332 13.6588 22.496 14.9921 21.7343L21.9897 17.7367C23.3336 16.969 23.3336 15.0313 21.9897 14.2635L14.9921 10.266Z\" fill=\"#ffffff\"/> </svg>";
char const svgreplay_glow[] = "<svg width=\"200\" height=\"200\" xmlns=\"http://www.w3.org/2000/svg\"> <!-- Define the glow filter --> <defs> <filter id=\"glow\" x=\"-20%\" y=\"-20%\" width=\"200%\" height=\"200%\"> <!-- Apply Gaussian blur to create the glow effect --> <feGaussianBlur stdDeviation=\"4\" result=\"coloredBlur\"/> <!-- Change color of the glow --> <feFlood flood-color=\"blue\" result=\"floodedBlur\"/> <feComposite in=\"floodedBlur\" in2=\"coloredBlur\" operator=\"in\" result=\"coloredBlur\"/> <!-- Merge the original graphic with the blurred glow --> <feMerge> <feMergeNode in=\"coloredBlur\"/> <feMergeNode in=\"SourceGraphic\"/> </feMerge> </filter> </defs> <!-- Your SVG content here --> <!-- Apply the glow filter to the desired element --> <path d=\"M7.05557 8C9.25286 5.545 12.446 4 16 4C22.6274 4 28 9.37258 28 16C28 22.6274 22.6274 28 16 28C9.37258 28 4 22.6274 4 16C4 15.7173 4.00978 15.4368 4.02902 15.1589C4.07121 14.5495 3.60935 14 2.99844 14C2.50687 14 2.08625 14.3612 2.04646 14.8512C2.01569 15.2301 2 15.6132 2 16C2 23.732 8.26801 30 16 30C23.732 30 30 23.732 30 16C30 8.26801 23.732 2 16 2C12.0826 2 8.54103 3.60893 6 6.20203V4C6 3.44772 5.55228 3 5 3C4.44772 3 4 3.44772 4 4V9C4 9.55228 4.44772 10 5 10H10C10.5523 10 11 9.55228 11 9C11 8.44772 10.5523 8 10 8H7.05557ZM14.9921 10.266C13.6588 9.5043 12 10.467 12 12.0026V19.9977C12 21.5332 13.6588 22.496 14.9921 21.7343L21.9897 17.7367C23.3336 16.969 23.3336 15.0313 21.9897 14.2635L14.9921 10.266Z\" fill=\"#ffffff\" filter=\"url(#glow)\"/> </svg>";

void
init_button(void)
{
	Button *b = {0};
	/* note: auto layout */
	b = malloc(sizeof(Button) * 4);
	b->count = 0;

	b[0].rect =(SDL_FRect){ .x = 50, .y = WINDOW_HEIGHT - 150, .w = 100.0f, .h = 100.0f};
	b[0].color = (SDL_Color) {.r = 170, .g = 50, .b = 50, .a = 255};
	b[0].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	b[0].hovered = false;
	b[0].pressed = false;
	b[0].released = false;
	b[0].fn = stop;
	b[0].text[0] = init_svg(svgstop, 100, 100);
	b[0].text[1] = init_svg(svgstop, 100, 100);

	b[1].rect =(SDL_FRect){ .x = b[0].rect.x + 150, .y = WINDOW_HEIGHT - 150, .w = 100.0f, .h = 100.0f};
	b[1].color = (SDL_Color) {.r = 170, .g = 50, .b = 0, .a = 255};
	b[1].color_pressed = (SDL_Color) {.r = 100, .g = 150, .b = 50, .a = 255};
	b[1].hovered = false;
	b[1].pressed = false;
	b[1].released = false;
	b[1].fn = my_toggle_play;
	b[1].text[0] = init_svg(svgplay, 100, 100);
	b[1].text[1] = init_svg(svgplay, 100, 100);

	b[2].rect =(SDL_FRect){ .x = b[1].rect.x + 150, .y = WINDOW_HEIGHT - 150, .w = 100.0f, .h = 100.0f};
	b[2].color = (SDL_Color) {.r = 170, .g = 100, .b = 50, .a = 255};
	b[2].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	b[2].hovered = false;
	b[2].pressed = false;
	b[2].released = false;
	b[2].fn = replay;
	b[2].text[0] = init_svg(svgreplay, 100, 100);
	b[2].text[1] = init_svg(svgreplay, 100, 100);

	b[3].rect =(SDL_FRect){ .x = b[2].rect.x + 150, .y = WINDOW_HEIGHT - 150, .w = 100.0f, .h = 100.0f};
	b[3].color = (SDL_Color) {.r = 170, .g = 100, .b = 50, .a = 255};
	b[3].color_pressed = (SDL_Color) {.r = 100, .g = 200, .b = 50, .a = 255};
	b[3].hovered = false;
	b[3].pressed = false;
	b[3].released = false;
	b[3].fn = test;
	b[3].text[0] = init_svg(rscreen_filled, 100, 100);
	b[3].text[1] = init_svg(fscreen_filled, 100, 100);

	b->count = 4;
	g_inst.buttons = b;
}
