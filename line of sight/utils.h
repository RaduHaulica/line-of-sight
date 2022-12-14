#pragma once

// ===== ===== =====
// UTILITY FUNCTIONS
// ===== ===== =====

const float pi = 3.141592f;

// utility function for quicksort
template <typename T>
int partition(std::vector<T>& v, int si, int ei, int (*f)(T, T))
{
    T pivot = v[ei];
    int i = (si - 1);
    for (int j = si; j <= ei - 1; j++)
    {
        if ((*f)(v[j], pivot) > 0)
        {
            i++;
            std::swap(v[i], v[j]);
        }
    }
    std::swap(v[i + 1], v[ei]);
    return (i + 1);
}

template <typename T>
void quicksort(std::vector<T>& v, int si, int ei, int (*f)(T, T))
{
    if (si < ei)
    {
        int pi = partition<T>(v, si, ei, f);

        quicksort(v, si, pi - 1, f);
        quicksort(v, pi + 1, ei, f);
    }
}

struct Segment
{
    sf::Vector2f startPoint;
    sf::Vector2f endPoint;
};

float distanceBetweenPoints(sf::Vector2f v1, sf::Vector2f v2)
{
    return std::sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
}

float dot(sf::Vector2f v1, sf::Vector2f v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

float norm(sf::Vector2f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(sf::Vector2f v)
{
    return v / norm(v);
}

float cross2D(sf::Vector2f v1, sf::Vector2f v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

// 0 -> 2*pi range
// value based on cos of angle, so
// 0 -> pi = 1 to -1
// pi -> 2*pi = -1 to 1
// we ned to figure out if we're below
float computeAngleBetweenVectors360(sf::Vector2f v1, sf::Vector2f v2)
{
    sf::Vector2f rotated = { -v2.y, v2.x };
    v1 = normalize(v1);
    v2 = normalize(v2);
    if (dot(v1, rotated) > 0)
    {
        return std::acos(dot(v1, v2));
    }
    else
    {
        return 2 * pi - std::acos(dot(v1, v2));
    }
}

// utility function for comparison
// angles are measured based on value of cos on unit circle
// 0 -> pi = 1 -> -1
// pi -> 2*pi = -1 -> 1
int isFirstAngleSmaller(sf::Vector2f v1, sf::Vector2f v2)//, sf::Vector2f baseline = { 1,0 })
{
    sf::Vector2f baseline = { 1, 0 };
    baseline = normalize(baseline);
    float angle1 = computeAngleBetweenVectors360(baseline, v1);
    float angle2 = computeAngleBetweenVectors360(baseline, v2);

    if (angle1 < angle2) return -1; else return 1;
}

sf::Vector2f rotateVector(sf::Vector2f v, float angle)
{
    //float oldAngle = std::acos(normalize(v).x);
    float oldAngle = atan2(v.y, v.x);
    return sf::Vector2f({ norm(v) * cos(oldAngle + angle), norm(v) * sin(oldAngle + angle) });
}

sf::Vector2f raySegmentIntersectionPoint(sf::Vector2f origin, sf::Vector2f ray, sf::Vector2f s1, sf::Vector2f s2)
{
    sf::Vector2f solution = origin;
    sf::Vector2f rotated = { -(s2 - s1).y, (s2 - s1).x };
    solution += (dot(s1 - origin, normalize(rotated))) / (dot(normalize(rotated), ray)) * ray;

    return solution;
}

bool rayInstersectsSegment(sf::Vector2f origin, sf::Vector2f ray, sf::Vector2f s1, sf::Vector2f s2)
{
    // s2-s1 = AB

    // check if ray origin collinear with AB
    if (abs(cross2D(origin - s1, s2 - s1)) - 0.01f < 0)
    {
        return false;
    }

    sf::Vector2f I = raySegmentIntersectionPoint(origin, ray, s1, s2);
    // check if intersection point is A or B
    //if (I == s1 || I == s2)
    //{
    //	return false;
    //}

    // check if intersection point is outside of AB
    float ratio = dot(I - s1, I - s1) / dot(I - s1, s2 - s1); // I = point where ray intersects segment AB, ratio = AI/AB
    if (ratio < 0 || ratio > 1)
    {
        return false;
    }

    // check if intersection point in opposite direction to ray
    sf::Vector2f rotated = { -(s2 - s1).y, (s2 - s1).x }; // AB rotated by 90 deg
    if (-(dot(origin - s1, normalize(rotated))) / (dot(normalize(rotated), ray)) < 0)
    {
        return false;
    }

    return true;
}

// check (s1, s2) and (s1, point) slope to establish collinearity OR cross product must be zero
// then min(s1, s2) < coordinate(point) < max(s1, s2) to see if it's on the segment
bool isPointOnSegment(sf::Vector2f point, sf::Vector2f s1, sf::Vector2f s2)
{
    if (cross2D(s1 - point, s1 - s2) == 0)
        return false;
    if (
        point.x >= std::fmin(s1.x, s2.x) && point.x <= std::fmax(s1.x, s2.x)
        && (point.y >= std::fmin(s1.y, s2.y) && point.y <= std::fmax(s1.y, s2.y))
        )
        return true;
    else
        return false;
}

void printVectors(std::vector<sf::Vector2f> v)
{
    for (int i = 0; i < v.size(); i++)
    {
        std::cout << "(" << v[i].x << "," << v[i].y << ") ";
    }
    std::cout << std::endl;
}

void printArray(std::vector<float> v)
{
    for (int i = 0; i < v.size(); i++)
    {
        std::cout << v[i] << ", ";
    }
    std::cout << std::endl;
}

void printAngles(std::vector<sf::Vector2f> v)
{
    for (int i = 0; i < v.size(); i++)
    {
        std::cout << computeAngleBetweenVectors360(v[i], { 1,0 }) << ", ";
    }
    std::cout << std::endl;
}

bool isRightOfSegment(sf::Vector2f origin1, sf::Vector2f origin2, sf::Vector2f point)
{
    sf::Vector2f segment = { -(origin2.y - origin1.y), origin2.x - origin1.x }; // B-A then rotate left
    sf::Vector2f movedPoint = point - origin1; // C-A
    float dotProduct = segment.x * movedPoint.x + segment.y * movedPoint.y;
    if (dotProduct > 0)
    {
        //std::cout << "right" << std::endl;
        return true;
    }
    //std::cout << "left" << std::endl;
    return false;
}

bool insideTriangle(std::vector<sf::Vector2f> triangle, sf::Vector2f point)
{
    if (isRightOfSegment(triangle[0], triangle[1], point))
    {
        if (!isRightOfSegment(triangle[1], triangle[2], point))
            return false;
        if (!isRightOfSegment(triangle[2], triangle[0], point))
            return false;
        return true;
    }
    else
    {
        if (isRightOfSegment(triangle[1], triangle[2], point))
            return false;
        if (isRightOfSegment(triangle[2], triangle[0], point))
            return false;
        return true;
    }
}

std::vector<Segment> getSegmentsFromPolygon(sf::ConvexShape s)
{
    std::vector<Segment> solution;
    if (s.getPointCount() > 1)
    {
        int i = 0;
        Segment seg;
        while (i < s.getPointCount() - 1)
        {
            seg.startPoint = s.getPoint(i);
            i++;
            seg.endPoint = s.getPoint(i);
            solution.push_back(seg);
        }
        seg.startPoint = s.getPoint(i);
        seg.endPoint = s.getPoint(0);
        solution.push_back(seg);
    }
    return solution;
}

bool isPointInsideConvexPolygon(sf::ConvexShape cs, sf::Vector2f point)
{
    std::vector<Segment> vs = getSegmentsFromPolygon(cs);
    bool side = isRightOfSegment(vs[0].startPoint, vs[0].endPoint, point);

    for (int i = 1; i < vs.size(); i++)
    {
        if (isRightOfSegment(vs[i].startPoint, vs[i].endPoint, point) != side)
        {
            return false;
        }
    }

    return true;
}

void loadShapes(std::vector<sf::ConvexShape>& shapes)
{
    sf::RectangleShape shape2;
    shape2 = sf::RectangleShape();
    sf::ConvexShape shape;
    shape.setFillColor(sf::Color::Blue);
    // top left
    shape.setPointCount(4);
    shape.setPoint(0, sf::Vector2f({ 100, 100 }));
    shape.setPoint(1, sf::Vector2f({ 350, 100 }));
    shape.setPoint(2, sf::Vector2f({ 300, 200 }));
    shape.setPoint(3, sf::Vector2f({ 100, 150 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 100, 200 }));
    shape.setPoint(1, sf::Vector2f({ 250, 200 }));
    shape.setPoint(2, sf::Vector2f({ 200, 300 }));
    shape.setPoint(3, sf::Vector2f({ 100, 250 }));
    shapes.push_back(shape);

    // bottom left
    shape.setPoint(0, sf::Vector2f({ 100, 400 }));
    shape.setPoint(1, sf::Vector2f({ 200, 400 }));
    shape.setPoint(2, sf::Vector2f({ 200, 500 }));
    shape.setPoint(3, sf::Vector2f({ 100, 500 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 100, 500 }));
    shape.setPoint(1, sf::Vector2f({ 400, 500 }));
    shape.setPoint(2, sf::Vector2f({ 400, 600 }));
    shape.setPoint(3, sf::Vector2f({ 100, 600 }));
    shapes.push_back(shape);

    // middle top
    shape.setPoint(0, sf::Vector2f({ 400, 100 }));
    shape.setPoint(1, sf::Vector2f({ 500, 100 }));
    shape.setPoint(2, sf::Vector2f({ 500, 400 }));
    shape.setPoint(3, sf::Vector2f({ 400, 400 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 500, 300 }));
    shape.setPoint(1, sf::Vector2f({ 700, 300 }));
    shape.setPoint(2, sf::Vector2f({ 700, 400 }));
    shape.setPoint(3, sf::Vector2f({ 500, 400 }));
    shapes.push_back(shape);

    // middle bottom
    shape.setPoint(0, sf::Vector2f({ 500, 500 }));
    shape.setPoint(1, sf::Vector2f({ 600, 500 }));
    shape.setPoint(2, sf::Vector2f({ 600, 700 }));
    shape.setPoint(3, sf::Vector2f({ 500, 700 }));
    shapes.push_back(shape);

    // right top
    shape.setPoint(0, sf::Vector2f({ 800, 100 }));
    shape.setPoint(1, sf::Vector2f({ 1200, 100 }));
    shape.setPoint(2, sf::Vector2f({ 1200, 200 }));
    shape.setPoint(3, sf::Vector2f({ 800, 200 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 1100, 200 }));
    shape.setPoint(1, sf::Vector2f({ 1200, 200 }));
    shape.setPoint(2, sf::Vector2f({ 1200, 500 }));
    shape.setPoint(3, sf::Vector2f({ 1100, 500 }));
    shapes.push_back(shape);

    // right bottom
    shape.setPoint(0, sf::Vector2f({ 800, 400 }));
    shape.setPoint(1, sf::Vector2f({ 900, 400 }));
    shape.setPoint(2, sf::Vector2f({ 900, 500 }));
    shape.setPoint(3, sf::Vector2f({ 800, 500 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 800, 600 }));
    shape.setPoint(1, sf::Vector2f({ 900, 600 }));
    shape.setPoint(2, sf::Vector2f({ 900, 700 }));
    shape.setPoint(3, sf::Vector2f({ 800, 700 }));
    shapes.push_back(shape);

    shape.setPoint(0, sf::Vector2f({ 900, 400 }));
    shape.setPoint(1, sf::Vector2f({ 1000, 400 }));
    shape.setPoint(2, sf::Vector2f({ 1000, 700 }));
    shape.setPoint(3, sf::Vector2f({ 900, 700 }));
    shapes.push_back(shape);
}

void loadEdges(sf::ConvexShape& screenEdges)
{
    screenEdges.setPointCount(4);
    screenEdges.setPoint(0, { 0, 0 });
    screenEdges.setPoint(1, { 1600, 0 });
    screenEdges.setPoint(2, { 1600, 800 });
    screenEdges.setPoint(3, { 0, 800 });
}