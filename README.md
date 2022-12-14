360 line of sight of an actor in a maze with 2D geometry and moving unit detection

Thought how to go about implementing a vision cone for units and ended up creating a 360 vision prototype. As I started going through walls while testing the line of sight mechanics i decided to also implement a simple collision detection algorithm to keep things nice and clean.

Initially i thought about casting 360 rays around the player avatar and then figuring out where they intersect the scene geometry.

I divided the scene geometry into segments and looped through them for each ray, keeping the shortest distance for each ray in order to avoid wall penetration.

This seemed to be a good start, but as it turns out, in order to create a field of vision you need to create a polygon (or at least a pretty good illusion).

The next step is ordering all the ray intersection points based on the angle with the OX axis and then creating triangles (origin, point, next_point).

Filling all the triangles with a low alpha texture results in a "polygon" that mimics line of sight, but mine clipped corners so additional steps had to be taken.

Since I already had the scene geometry divided into segments, I decided that instead of just randomly casting rays in a circular pattern it would make more sense to cast them towards the points at the edges of these segments instead.

This was definitely an upgrade in creating a more precise field of vision but my rays were not going past corners. So for each ray cast toward a point I cast two extra rays rotated by 1 degree to the left and right of the original ray in order to cover the areas behind corners when creating the final line of sight polygon.

The end result looked pretty neat, and since I had all the triangles already calculated, I decided to throw in some random movement "enemies" that would bounce off the walls and would be highlighted when they intersected my field of vision, in order to better showcase how vision works.
