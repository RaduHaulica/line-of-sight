#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

// ========================
//      CLASSES
// ========================

class IUpdatable
{
public:
    virtual void update(float dt) = 0;
};

class Entity : public IUpdatable
{
public:
    sf::Vector2f position;
    sf::Vector2f lastPosition;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float max_speed;

    Entity(sf::Vector2f position, sf::Vector2f velocity = { 0,0 }, sf::Vector2f acceleration = { 0, 0 }, float max_speed = 200.0f) :
        position(position),
        lastPosition(position),
        velocity(velocity),
        acceleration(acceleration),
        max_speed(max_speed)
    {}

    virtual void update(float dt) override
    {
        lastPosition = position;

        velocity += acceleration * dt;
        position += velocity * dt;
    }
};

class IInputComponent
{
public:
    bool movingLeft;
    bool movingRight;
    bool movingUp;
    bool movingDown;
    bool moving;

    IInputComponent() :
        moving(false),
        movingLeft(false),
        movingRight(false),
        movingUp(false),
        movingDown(false)
    {
    }

    virtual void update(Entity& actor, float dt) = 0;
};

class KeyboardInput : public IInputComponent
{
public:
    KeyboardInput() :
        IInputComponent()
    {}

    void update(Entity& actor, float dt) override
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            movingLeft = true;
            moving = true;
        }
        else
        {
            movingLeft = false;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            movingRight = true;
            moving = true;
        }
        else
        {
            movingRight = false;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            movingUp = true;
            moving = true;
        }
        else
        {
            movingUp = false;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            movingDown = true;
            moving = true;
        }
        else
        {
            movingDown = false;
        }

        float speedMultiplier = 4.0f;

        if (movingLeft)
        {
            if (actor.velocity.x > 0.0f)
            {
                actor.velocity.x = 0.0f;
            }
            actor.acceleration = sf::Vector2f({ -speedMultiplier * actor.max_speed, actor.acceleration.y });
        }
        if (movingRight)
        {
            if (actor.velocity.x < 0.0f)
            {
                actor.velocity.x = 0.0f;
            }
            actor.acceleration = sf::Vector2f({ speedMultiplier * actor.max_speed, actor.acceleration.y });
        }
        if (movingUp)
        {
            if (actor.velocity.y > 0.0f)
            {
                actor.velocity.y = 0.0f;
            }
            actor.acceleration = sf::Vector2f({ actor.acceleration.x, -speedMultiplier * actor.max_speed });
        }
        if (movingDown)
        {
            if (actor.velocity.y < 0.0f)
            {
                actor.velocity.y = 0.0f;
            }
            actor.acceleration = sf::Vector2f({ actor.acceleration.x, speedMultiplier * actor.max_speed });
        }
        if (norm(actor.acceleration) > speedMultiplier * actor.max_speed)
        {
            actor.acceleration = normalize(actor.acceleration) * speedMultiplier * actor.max_speed;
        }

        if (!movingLeft && !movingRight)
        {
            actor.acceleration.x = -speedMultiplier * actor.velocity.x;
        }

        if (!movingUp && !movingDown)
        {
            actor.acceleration.y = -speedMultiplier * actor.velocity.y;
        }

    }
};

class CollisionCircle
{
public:
    CollisionCircle(sf::Vector2f origin, float radius = 3.0f) :
        origin(origin),
        radius(radius)
    {}
private:
    float radius;
    sf::Vector2f origin;
};

class Game;
class RayCaster;

class Enemy : public Entity, public sf::Drawable
{
    bool seen;
    sf::VertexArray collisionVA;
public:
    Enemy(sf::Vector2f pos, sf::Vector2f velocity = { 0,0 }, sf::Vector2f acceleration = { 0, 0 }) :
        Entity(pos, velocity, acceleration),
        seen(false)
    {
        collisionVA.setPrimitiveType(sf::PrimitiveType::Lines);
    }

    void update(Game& game, RayCaster& player, float dt);

    virtual void draw(sf::RenderTarget& w, sf::RenderStates rs) const
    {
        sf::Color color = sf::Color::Magenta;
        if (seen)
        {
            color = sf::Color::Green;
        }
        sf::CircleShape cs = sf::CircleShape(10.0f, 20);
        cs.setFillColor(color);
        cs.setOrigin({ 10.0f, 10.0f });
        cs.setPosition(position);
        w.draw(cs);

        w.draw(collisionVA);

        sf::VertexArray va;
        va.append({ position, sf::Color::Black });
        w.draw(va);
    }
};

class Game : public sf::Drawable
{
public:
    std::vector<sf::ConvexShape> shapes;
    sf::ConvexShape screenEdges;
    std::vector<Enemy> enemies;

    void update(RayCaster& player, float dt)
    {
        for (int i = 0; i < enemies.size(); i++)
        {
            enemies[i].update(*this, player, dt);
        }
    }

    void init()
    {
        loadShapes(this->shapes);
        loadEdges(this->screenEdges);
        enemies.push_back(Enemy({ 250.0f, 250.0f }, { 210, 16 }));
        enemies.push_back(Enemy({ 550.0f, 250.0f }, { 190, 87 }));
        enemies.push_back(Enemy({ 850.0f, 550.0f }, { -278, -34 }));
        enemies.push_back(Enemy({ 450.0f, 550.0f }, { -135, -63 }));
    }

    virtual void draw(sf::RenderTarget& window, sf::RenderStates) const override
    {
        for (int i = 0; i < shapes.size(); i++)
        {
            window.draw(shapes[i]);
        }
        for (int i = 0; i < enemies.size(); i++)
        {
            window.draw(enemies[i]);
        }
    }
};

class IPhysicsComponent
{
public:
    virtual void update(Game& game, RayCaster& actor, float dt) = 0;
};

class PhysicsComponent : public IPhysicsComponent
{
public:
    virtual void update(Game& game, RayCaster& actor, float dt) override;
};

class RayCaster : public Entity, public sf::Drawable
{
public:
    int raysAmount;
    std::vector<sf::Vector2f> rays, collisionPointsRelative;
    std::vector<Segment> collisionSegments;
    std::vector<float> collisionDistances;
    sf::VertexArray raysVA, visionVA, origin, collisionSegmentsVA, collisionEdgeVA;
    sf::CircleShape sprite;
    std::vector<sf::CircleShape> collisionPointsCircles;
    IInputComponent* inputComponent;
    IPhysicsComponent* physicsComponent;
    sf::Vector2f nearestPoint;
    Segment nearestSegment;
    float nearestDistance;
    Segment collisionEdge;

    RayCaster(sf::Vector2f position, int rays) :
        Entity(position),
        raysAmount(rays)
    {
        this->generateRadialRays(rays);

        this->raysVA.setPrimitiveType(sf::PrimitiveType::Lines);
        this->visionVA.setPrimitiveType(sf::PrimitiveType::Triangles);
        this->collisionSegmentsVA.setPrimitiveType(sf::PrimitiveType::Lines);
        this->collisionEdgeVA.setPrimitiveType(sf::PrimitiveType::Lines);

        sprite = sf::CircleShape(10, 20);
        sprite.setFillColor(sf::Color::Red);
        sprite.setPosition(position);
        sprite.setOrigin({ 10.0f , 10.0f });

        origin.setPrimitiveType(sf::PrimitiveType::Points);
        origin.append({ position, sf::Color::Black });


        inputComponent = new KeyboardInput();
        physicsComponent = new PhysicsComponent();
    }

    void generateRadialRays(int amount)
    {
        raysAmount = amount;
        rays.clear();
        sf::Vector2f v{ 1, 0 };
        float offset = 2 * pi / raysAmount;
        for (int i = 0; i < raysAmount; i++)
        {
            this->rays.push_back(rotateVector(v, i * offset));
        }
    }

    void generateVisionRays(Game& g)
    {
        rays.clear();
        for (int i = 0; i < g.shapes.size(); i++)
        {
            for (int j = 0; j < g.shapes[i].getPointCount(); j++)
            {
                sf::Vector2f ray = normalize(g.shapes[i].getPoint(j) - position);
                rays.push_back(rotateVector(ray, 0.001f));
                rays.push_back(ray);
                rays.push_back(rotateVector(ray, -0.001f));
            }
        }

        for (int j = 0; j < g.screenEdges.getPointCount(); j++)
        {
            sf::Vector2f ray = normalize(g.screenEdges.getPoint(j) - position);
            rays.push_back(rotateVector(ray, 0.001f));
            rays.push_back(ray);
            rays.push_back(rotateVector(ray, -0.001f));
        }

        raysAmount = rays.size();
    }

    void update(sf::Window& window, Game& game, float dt)
    {
        inputComponent->update(*this, dt);
        if (norm(velocity) > max_speed)
        {
            velocity = normalize(velocity) * max_speed;
        }
        Entity::update(dt);
        physicsComponent->update(game, *this, dt);

        sprite.setPosition(position);
        //this->position = sf::Vector2f(sf::Mouse::getPosition(window));
        this->origin.clear();
        origin.append({ position, sf::Color::Black });

        this->raysVA.clear();
        this->visionVA.clear();
        this->collisionSegmentsVA.clear();
        this->collisionEdgeVA.clear();

        this->collisionPointsCircles.clear();
        this->collisionPointsRelative.clear();
        this->collisionDistances.clear();

        sf::Color color = sf::Color::White;
        color.a = 32;

        this->generateVisionRays(game); // create line of sight with geometry corners

        nearestDistance = FLT_MAX;
        for (int i = 0; i < rays.size(); i++)
        {
            sf::Vector2f nearestCollisionPoint;
            Segment nearestSegment;
            float nearestCollisionDistance = FLT_MAX;

            for (int j = 0; j < game.shapes.size(); j++)
            {
                std::vector<Segment> segments = getSegmentsFromPolygon(game.shapes[j]);

                for (int k = 0; k < segments.size(); k++)
                {
                    if (rayInstersectsSegment(position, rays[i], segments[k].startPoint, segments[k].endPoint))
                    {
                        sf::Vector2f currentCollisionPoint = raySegmentIntersectionPoint(position, rays[i], segments[k].startPoint, segments[k].endPoint);
                        float distance = distanceBetweenPoints(position, currentCollisionPoint);
                        if (distance < nearestCollisionDistance)
                        {
                            nearestCollisionDistance = distance;
                            nearestCollisionPoint = currentCollisionPoint;
                            nearestSegment.startPoint = segments[k].startPoint;
                            nearestSegment.endPoint = segments[k].endPoint;
                        }
                    }

                }
            }

            if (nearestCollisionDistance == FLT_MAX)
            {
                std::vector<Segment> segments = getSegmentsFromPolygon(game.screenEdges);

                for (int k = 0; k < segments.size(); k++)
                {
                    if (rayInstersectsSegment(position, rays[i], segments[k].startPoint, segments[k].endPoint))
                    {
                        sf::Vector2f currentCollisionPoint = raySegmentIntersectionPoint(position, rays[i], segments[k].startPoint, segments[k].endPoint);
                        float distance = distanceBetweenPoints(position, currentCollisionPoint);
                        if (distance < nearestCollisionDistance)
                        {
                            nearestCollisionDistance = distance;
                            nearestCollisionPoint = currentCollisionPoint;
                            nearestSegment.startPoint = segments[k].startPoint;
                            nearestSegment.endPoint = segments[k].endPoint;
                        }
                    }

                }
            }

            collisionPointsRelative.push_back(nearestCollisionPoint - position);
            collisionDistances.push_back(nearestCollisionDistance);

            if (nearestCollisionDistance < nearestDistance)
            {
                nearestDistance = nearestCollisionDistance;
                nearestPoint = nearestCollisionPoint;
                collisionEdge = nearestSegment;
            }

            if (nearestCollisionPoint != nearestSegment.startPoint && nearestCollisionPoint != nearestSegment.endPoint)
            {
                collisionSegments.push_back(nearestSegment);
                collisionSegmentsVA.append({ nearestSegment.startPoint, sf::Color::Green });
                collisionSegmentsVA.append({ nearestSegment.endPoint, sf::Color::Green });
            }

            raysVA.append({ position, color });
            raysVA.append({ nearestCollisionPoint, color });

            sf::CircleShape cs(3, 20);
            cs.setPosition(nearestCollisionPoint);
            cs.setFillColor(sf::Color::Green);
            cs.setOrigin({ 3, 3 });
            collisionPointsCircles.push_back(cs);
        }

        bool firstPoint{ true };
        sf::Vector2f lastPoint;
        quicksort<sf::Vector2f>(collisionPointsRelative, 0, collisionPointsRelative.size() - 1, isFirstAngleSmaller);
        for (int i = 0; i < collisionPointsRelative.size(); i++)
        {
            collisionPointsRelative[i] += position;
        }
        for (int i = 0; i < collisionPointsRelative.size(); i++)
        {
            if (firstPoint)
            {
                firstPoint = false;
                lastPoint = collisionPointsRelative[i];
            }
            else
            {
                this->visionVA.append({ position, color });
                this->visionVA.append({ lastPoint, color });
                this->visionVA.append({ collisionPointsRelative[i], color });
                lastPoint = collisionPointsRelative[i];
            }
        }
        this->visionVA.append({ position, color });
        this->visionVA.append({ lastPoint, color });
        this->visionVA.append({ visionVA[1].position, color });
        this->collisionEdgeVA.append({ collisionEdge.startPoint, sf::Color::Red });
        this->collisionEdgeVA.append({ collisionEdge.endPoint, sf::Color::Red });
        //system("PAUSE");
    }

    virtual void draw(sf::RenderTarget& w, sf::RenderStates rs) const
    {
        w.draw(this->visionVA);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            w.draw(this->raysVA);
            for (int i = 0; i < collisionPointsCircles.size(); i++)
            {
                w.draw(collisionPointsCircles[i]);
            }
            w.draw(collisionSegmentsVA);
        }
        w.draw(sprite);
        w.draw(origin);
        w.draw(collisionEdgeVA);
    }
};

void PhysicsComponent::update(Game& game, RayCaster& actor, float dt)
{
    float radius = 10.0f;
    bool collisionDetected{ false };

    for (int i = 0; i < game.shapes.size(); i++)
    {
        // left
        //if (game.shapes[i].getGlobalBounds().contains(actor.position + sf::Vector2f({ -radius, 0.0f })))
        if (isPointInsideConvexPolygon(game.shapes[i], (actor.position + sf::Vector2f({ -radius, 0.0f }))))
        {
            //actor.position.x += 1.0f; // game.shapes[i].getGlobalBounds().left + game.shapes[i].getGlobalBounds().width + radius;
            collisionDetected = true;
            actor.acceleration.x = 0.0f;
            actor.velocity.x = 0.0f;
        }
        // right
        //if (game.shapes[i].getGlobalBounds().contains(actor.position + sf::Vector2f({ radius, 0.0f })))
        if (isPointInsideConvexPolygon(game.shapes[i], (actor.position + sf::Vector2f({ radius, 0.0f }))))
        {
            //actor.position.x -= 1.0f; // game.shapes[i].getGlobalBounds().left - radius;
            collisionDetected = true;
            actor.acceleration.x = 0.0f;
            actor.velocity.x = 0.0f;
        }
        // up
        //if (game.shapes[i].getGlobalBounds().contains(actor.position + sf::Vector2f({ 0.0f, -radius })))
        if (isPointInsideConvexPolygon(game.shapes[i], (actor.position + sf::Vector2f({ 0.0f, -radius }))))
        {
            //actor.position.y += 1.0f; // game.shapes[i].getGlobalBounds().top + game.shapes[i].getGlobalBounds().height + radius;
            collisionDetected = true;
            actor.acceleration.y = 0.0f;
            actor.velocity.y = 0.0f;
        }
        // down
        //if (game.shapes[i].getGlobalBounds().contains(actor.position + sf::Vector2f({ 0.0f, radius })))
        if (isPointInsideConvexPolygon(game.shapes[i], (actor.position + sf::Vector2f({ 0.0f, radius }))))
        {
            //actor.position.y -= 1.0f; // game.shapes[i].getGlobalBounds().top - radius;
            collisionDetected = true;
            actor.acceleration.y = 0.0f;
            actor.velocity.y = 0.0f;
        }
    }

    if (collisionDetected)
    {
        int minIndex = 0;
        float minDistance = actor.collisionDistances[0];
        for (int i = 1; i < actor.collisionDistances.size(); i++)
        {
            if (actor.collisionDistances[i] < minDistance)
            {
                minIndex = i;
                minDistance = actor.collisionDistances[i];
            }
        }

        sf::Vector2f normalVector = { (actor.collisionEdge.endPoint - actor.collisionEdge.startPoint).y, -(actor.collisionEdge.endPoint - actor.collisionEdge.startPoint).x };
        normalVector = normalize(normalVector);
        if (dot(normalVector, actor.velocity) > 0)
        {
            normalVector = -normalVector;
        }
        sf::Vector2f newPosition;
        sf::Vector2f segmentRotated = (actor.collisionEdge.endPoint - actor.collisionEdge.startPoint);
        segmentRotated = sf::Vector2f({ segmentRotated.y, -segmentRotated.x });
        sf::Vector2f projection; // = actor.position + dot((actor.position - actor.collisionEdge.startPoint), normalize(segmentRotated)) * (-normalize(segmentRotated));
        if (rayInstersectsSegment(actor.position, -normalize(segmentRotated), actor.collisionEdge.startPoint, actor.collisionEdge.endPoint))
        {
            projection = raySegmentIntersectionPoint(actor.position, -normalize(segmentRotated), actor.collisionEdge.startPoint, actor.collisionEdge.endPoint);
            newPosition = actor.collisionEdge.startPoint + norm(actor.position - actor.collisionEdge.startPoint) * (dot(projection - actor.collisionEdge.startPoint, actor.position - actor.collisionEdge.startPoint) / (norm(projection - actor.collisionEdge.startPoint) * norm(actor.position - actor.collisionEdge.startPoint))) * normalize(actor.collisionEdge.endPoint - actor.collisionEdge.startPoint) + 1.05f * radius * normalVector;
            actor.position = newPosition;
        }

    }

}

void Enemy::update(Game& game, RayCaster& player, float dt)
{
    std::cout <<"("<< this->position.x << ", " << this->position.y <<")" << std::endl;
    // collision and change direction
    collisionVA.clear();
    std::vector<sf::ConvexShape> allWalls;
    for (int i = 0; i < game.shapes.size(); i++)
    {
        allWalls.push_back(game.shapes[i]);
    }
    //allWalls.push_back(game.screenEdges);
    if (position.x < 0 || position.x > 1600) velocity.x = -velocity.x;
    if (position.y < 0 || position.y > 800) velocity.y = -velocity.y;

    for (int i = 0; i < allWalls.size(); i++)
    {
        if (isPointInsideConvexPolygon(allWalls[i], (position + normalize(velocity) * 10.0f + velocity * dt + acceleration * dt)))
        {
            Segment collisionEdge;
            float nearestCollision{ FLT_MAX };
            sf::Vector2f nearestProjection;
            sf::Vector2f nearestIntersection;
            for (int j = 0; j < allWalls[i].getPointCount(); j++)
            {
                sf::Vector2f ray = normalize(allWalls[i].getPoint((j + 1) % allWalls[i].getPointCount()) - allWalls[i].getPoint(j));
                ray = { -ray.y, ray.x };
                sf::Vector2f projection = raySegmentIntersectionPoint(position, ray, allWalls[i].getPoint(j), allWalls[i].getPoint((j + 1) % allWalls[i].getPointCount()));
                //if (isPointOnSegment(projection, allWalls[i].getPoint(j), allWalls[i].getPoint((j + 1) % allWalls[i].getPointCount())))
				if (true)
                {
                    if (distanceBetweenPoints(position, projection) < nearestCollision)
                    {
                        nearestCollision = distanceBetweenPoints(position, projection);
                        collisionEdge.startPoint = allWalls[i].getPoint(j);
                        collisionEdge.endPoint = allWalls[i].getPoint((j + 1) % allWalls[i].getPointCount());
                        nearestProjection = projection;
                        nearestIntersection = raySegmentIntersectionPoint(position, velocity, collisionEdge.startPoint, collisionEdge.endPoint);
                    }
                }
            }

            acceleration = { 0, 0 };
            float oldVelocity = norm(velocity);
            float alpha = std::asin(distanceBetweenPoints(lastPosition, nearestProjection) / distanceBetweenPoints(lastPosition, nearestIntersection));
            sf::Vector2f normalVector = rotateVector(normalize(collisionEdge.endPoint - collisionEdge.startPoint), pi / 2);
            if (dot(normalVector, velocity) < 0)
            {
                normalVector = -normalVector;
            }
            velocity = velocity - 2 * dot(normalVector, velocity) * normalVector;
            //velocity = norm(velocity) * 1.05f * normalize(velocity);
            collisionVA.append({ collisionEdge.startPoint, sf::Color::Red });
            collisionVA.append({ collisionEdge.endPoint, sf::Color::Red });
            //if (collisionEdge.startPoint.x == collisionEdge.endPoint.x) velocity.x = -velocity.x;
            //if (collisionEdge.startPoint.y == collisionEdge.endPoint.y) velocity.y = -velocity.y;

            break; // stop looking through other polygons
        }
    }

    // are the enemies inside the vision polygon
    this->Entity::update(dt);
    seen = false;
    for (int i = 0; i < player.visionVA.getVertexCount(); i++)
    {
        std::vector<sf::Vector2f> v;
        v.push_back(player.visionVA[i++].position);
        v.push_back(player.visionVA[i++].position);
        v.push_back(player.visionVA[i].position);
        if (
            insideTriangle(v, position + sf::Vector2f({ -10.0f, 0 }))
            || insideTriangle(v, position + sf::Vector2f({ 10.0f, 0 }))
            || insideTriangle(v, position + sf::Vector2f({ 0.0f, -10.0f }))
            || insideTriangle(v, position + sf::Vector2f({ 0.0f, 10.0f }))
            )
        {
            seen = true;
            break;
        }
    }
}
// ====================
//    THE MAIN THING
// ====================

int main()
{
    sf::RenderWindow window(sf::VideoMode(1600, 800), "SFML works!");
    sf::View camera;
    camera.setCenter(800, 400);
    camera.setSize(1600, 800);
    window.setView(camera);

    sf::Font font;
    font.loadFromFile("./Roboto-Bold.ttf");
    sf::Text helpText;
    helpText.setFont(font);
    helpText.setCharacterSize(12);
    helpText.setFillColor(sf::Color::White);
    helpText.setString("Dynamic line of sight and visible object detection\nEdges highlighted on collision\nClosest edge to player highlighted\nPress Space to see vision lines\n\nArrow keys for movement\nPress P to pause");
    helpText.setPosition({ 0, 0 });

    sf::Clock clock;
    float dt;

    sf::Vector2f mPosGlobal;
    sf::Vector2f mPos;

    sf::Vector2f rayOrigin{ 800,400 };
    sf::Vector2f rayDestination;
    sf::Vector2f movable;
    sf::VertexArray vaLines, vaPoints;
    vaLines.setPrimitiveType(sf::PrimitiveType::Lines);
    vaPoints.setPrimitiveType(sf::PrimitiveType::Points);

    Game game;
    game.init();

    RayCaster player({ 775, 375 }, 360);

    bool drawRay{ true };
    bool pause{ false };
    float inputLockDuration{ 0.2f };
    float inputLockElapsed{ 0.0f };

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // timer
        dt = clock.restart().asSeconds();

        // input
        mPos = (sf::Vector2f)sf::Mouse::getPosition(window);
        movable = mPos;

        system("CLS");
        //std::cout << "Mouse x:" << mPos.x << ", y:" << mPos.y << std::endl;

        if (inputLockElapsed > 0)
        {
            inputLockElapsed -= dt;
            if (inputLockElapsed < 0) inputLockElapsed = 0.0f;
        }
        else
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                drawRay = !drawRay;
                inputLockElapsed = inputLockDuration;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            {
                inputLockElapsed = inputLockDuration;
                pause = !pause;
            }
        }

        // do stuff
        if (pause)
        {
            continue;
        }
        player.update(window, game, dt);
        game.update(player, dt);

        // prepare graphics
        vaLines.clear();
        vaPoints.clear();

        // render
        window.clear();

        window.draw(game);

        window.draw(vaLines);
        //std::cout << "rays: " << player.raysAmount << " in array: " << player.rays.size() << std::endl;
        //std::cout << "ray vertexes: " << player.raysVA.getVertexCount() << std::endl;
        window.draw(vaPoints);

        window.draw(player);

        window.draw(helpText);

        window.display();
    }

    return 0;
}