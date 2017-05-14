/*
 * Copyright (c) scott.cgi All Rights Reserved.
 *
 * Since : 2014-7-23
 * Author: scott.cgi
 */

#include "Engine/Physics/PhysicsCollision.h"
#include "Engine/Toolkit/Platform/Log.h"


/**
 * Test polygonA each vertex in polygonB, true inside or false outside
 * can test through and cross each others
 */
static inline bool TestPolygonPolygonFull(Array(float)* polygonA, Array(float)* polygonB)
{
	int leftCount  = 0;
	int rightCount = 0;

	for (int i = 0; i < polygonA->length; i += 2)
	{
		float x        = AArrayGet(polygonA, i,     float);
		float y        = AArrayGet(polygonA, i + 1, float);

		int   preIndex = polygonB->length - 2;

		// test polygonB contains vertex
		for (int j = 0; j < polygonB->length; j += 2)
		{
			float vertexY = AArrayGet(polygonB, j        + 1, float);
			float preY    = AArrayGet(polygonB, preIndex + 1, float);

			if ((vertexY < y && preY >= y) || (preY < y && vertexY >= y))
			{
				float vertexX = AArrayGet(polygonB, j, float);

				// cross product between vector (x - vertexX, y - vertexY) and (preX - vertexX, preY - vertexY)
				// result is (x - vertexX) * (preY - vertexY) - (y - vertexY) * (preX - vertexX)
				// if result zero means point (x, y) on vector (preX - vertexX, preY - vertexY)
				// if result positive means point on left  vector
				// if result negative means point on right vector
				if (vertexX + (y - vertexY) / (preY - vertexY) * (AArrayGet(polygonB, preIndex, float) - vertexX) <= x)
				{
					leftCount++;
				}
				else
				{
					rightCount++;
				}
			}

			preIndex = j;
		}

		if (leftCount % 2 != 0)
		{
			return true;
		}
	}

	return leftCount != 0 && leftCount == rightCount;
}


/**
 * Test polygonA each vertex in polygonB, true inside or false outside
 * not test through and cross each others
 */
static inline bool TestPolygonPolygon(Array(float)* polygonA, Array(float)* polygonB)
{
	bool inside = false;

	for (int i = 0; i < polygonA->length; i += 2)
	{
		float x        = AArrayGet(polygonA, i,     float);
		float y        = AArrayGet(polygonA, i + 1, float);
		int   preIndex = polygonB->length - 2;

		// test polygonB contains vertex
		for (int j = 0; j < polygonB->length; j += 2)
		{
			float vertexY = AArrayGet(polygonB, j        + 1, float);
			float preY    = AArrayGet(polygonB, preIndex + 1, float);

			if ((vertexY < y && preY >= y) || (preY < y && vertexY >= y))
			{
				float vertexX = AArrayGet(polygonB, j, float);

				// cross product between vector (x - vertexX, y - vertexY) and (preX - vertexX, preY - vertexY)
				// result is (x - vertexX) * (preY - vertexY) - (y - vertexY) * (preX - vertexX)
				// if result zero means point (x, y) on vector (preX - vertexX, preY - vertexY)
				// if result positive means point on left  vector
				// if result negative means point on right vector
				if (vertexX + (y - vertexY) / (preY - vertexY) * (AArrayGet(polygonB, preIndex, float) - vertexX) <= x)
				{
					inside = !inside;
				}
			}

			preIndex = j;
		}

		if (inside)
		{
			return true;
		}
	}

	return inside;
}


/**
 * Test one lineA intersect lineB
 */
static inline bool TestLineLine(Array(float)* lineA, Array(float)* lineB)
{
	int   flag[2]  = {0, 0};
	float vertexX1 = AArrayGet(lineB, 0, float);
	float vertexX2 = AArrayGet(lineB, 2, float);
	float vertexY1 = AArrayGet(lineB, 1, float);
	float vertexY2 = AArrayGet(lineB, 3, float);

	for (int i = 0; i < 4; i += 2)
	{
		float x = AArrayGet(lineA, i,     float);
		float y = AArrayGet(lineA, i + 1, float);

		if ((vertexY1 < y && vertexY2 >= y) || (vertexY2 < y && vertexY1 >= y))
		{
			// cross product between vector (x - vertexX1, y - vertexY1) and (vertexX2 - vertexX1, vertexY2 - vertexY1)
			// result is (x - vertexX1) * (vertexY2 - vertexY1) - (y - vertexY1) * (vertexX2 - vertexX1)
			if (vertexX1 + (y - vertexY1) / (vertexY2 - vertexY1) * (vertexX2 - vertexX1) <= x)
			{
				flag[i >> 1] = 1;
			}
			else
			{
				flag[i >> 1] = 2;
			}
		}
	}

	// test lineA two points both sides of lineB
	if (flag[0] + flag[1] == 3)
	{
		return true;
	}


	flag[0]  = 0;
	flag[1]  = 0;

	vertexX1 = AArrayGet(lineA, 0, float);
	vertexX2 = AArrayGet(lineA, 2, float);
	vertexY1 = AArrayGet(lineA, 1, float);
	vertexY2 = AArrayGet(lineA, 3, float);

	for (int i = 0; i < 4; i += 2)
	{
		float x = AArrayGet(lineB, i,     float);
		float y = AArrayGet(lineB, i + 1, float);

		if ((vertexY1 < y && vertexY2 >= y) || (vertexY2 < y && vertexY1 >= y))
		{
			// cross product between vector (x - vertexX1, y - vertexY1) and (vertexX2 - vertexX1, vertexY2 - vertexY1)
			// result is (x - vertexX1) * (vertexY2 - vertexY1) - (y - vertexY1) * (vertexX2 - vertexX1)
			if (vertexX1 + (y - vertexY1) / (vertexY2 - vertexY1) * (vertexX2 - vertexX1) <= x)
			{
				flag[i >> 1] = 1;
			}
			else
			{
				flag[i >> 1] = 2;
			}
		}
	}

	// test lineB two points both sides of lineA
	return flag[0] + flag[1] == 3;
}


/**
 * Test polygon contains point, true inside or false outside
 */
static inline bool TestPolygonPoint(Array(float)* polygon, Array(float)* point)
{
	bool   inside     = false;
	int    preIndex   = polygon->length - 2;
	float* vertexData = AArrayGetData(polygon, float);
	float  x          = AArrayGet(polygon, 0,  float);
	float  y          = AArrayGet(polygon, 1,  float);

	for (int i = 0; i < polygon->length; i += 2)
	{
		float vertexY = vertexData[i        + 1];
		float preY    = vertexData[preIndex + 1];

		if ((vertexY < y && preY >= y) || (preY < y && vertexY >= y))
		{
			float vertexX = vertexData[i];

			// cross product between vector (x - vertexX, y - vertexY) and (preX - vertexX, preY - vertexY)
			// result is (x - vertexX) * (preY - vertexY) - (y - vertexY) * (preX - vertexX)
			// if result zero means point (x, y) on vector (preX - vertexX, preY - vertexY)
			// if result positive means point on left  vector
			// if result negative means point on right vector
			if (vertexX + (y - vertexY) / (preY - vertexY) * (vertexData[preIndex] - vertexX) <= x)
			{
				inside = !inside;
			}
		}

		preIndex = i;
	}

	return inside;
}


//--------------------------------------------------------------------------------------------------


enum
{
	ploygon_ploygon = physics_shape_polygon | physics_shape_polygon,
	ploygon_line    = physics_shape_polygon | physics_shape_line,
	line_line       = physics_shape_line    | physics_shape_line,
	ploygon_point   = physics_shape_polygon | physics_shape_point,
};


static bool TestCollision(PhysicsBody* bodyA, PhysicsBody* bodyB)
{
	switch (bodyA->shape | bodyB->shape)
	{
		case ploygon_ploygon:
			return TestPolygonPolygon(bodyA->positionArr, bodyB->positionArr) || TestPolygonPolygon(bodyB->positionArr, bodyA->positionArr);

		case ploygon_line:
			// only consider line vertex in polygon
			if (bodyA->shape == physics_shape_line)
			{
				return TestPolygonPolygonFull(bodyA->positionArr, bodyB->positionArr);
			}
			else
			{
				return TestPolygonPolygonFull(bodyB->positionArr, bodyA->positionArr);
			}

		case line_line:
			return TestLineLine(bodyA->positionArr, bodyB->positionArr);

		case ploygon_point:
			// only consider point in polygon
			if (bodyA->shape == physics_shape_polygon)
			{
				return TestPolygonPoint(bodyA->positionArr, bodyB->positionArr);
			}
			else
			{
				return TestPolygonPoint(bodyB->positionArr, bodyA->positionArr);
			}
	}

	ALog_A(false, "Can not test collision between shape %d and %d", bodyA->shape, bodyB->shape);

	return false;
}


struct APhysicsCollision APhysicsCollision[1] =
{
	TestCollision,
};
