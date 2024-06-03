#ifndef VECTOR_H
#define VECTOR_H

typedef struct Vec2f
{
	float x, y;
}Vec2f;

Vec2f		vec2f(float x, float y);
Vec2f		vec2f_add(Vec2f x, Vec2f y);
Vec2f		vec2f_sub(Vec2f x, Vec2f y);
Vec2f		vec2f_mul(Vec2f x, Vec2f y);
Vec2f		vec2f_div(Vec2f x, Vec2f y);

#endif // vector.h
