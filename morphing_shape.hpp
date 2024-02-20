#ifndef _MORPHING_SHAPE_HPP_
#define _MORPHING_SHAPE_HPP_

#include <cmath>
#include <vector>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include "config.hpp"
#include "helper.hpp"

class MorphingShape {
private:
    sf::ConvexShape shape;
    sf::ConvexShape lastShape;
    sf::ConvexShape targetShape;
    float newShapeTimer {0.f};
    float newShapeTimerMax {1.f};

    void initShapes() {
        lastShape = generateRandomConvexShape();
        shape = lastShape;
        targetShape = generateRandomConvexShape();
    }

    sf::ConvexShape generateRandomConvexShape() {
        // following https://cglab.ca/~sander/misc/ConvexGeneration/convex.html to create random convex shapes
        // const unsigned pointCount {static_cast<unsigned>(rand() % 8 + 8)}; will make random later
        const unsigned pointCount {8};
        std::vector<float> randXVec;
        std::vector<float> randYVec;

        // create random x and y points
        for (size_t i {0}; i < pointCount; i++) {
            randXVec.push_back(static_cast<float>(rand() % 151));
            randYVec.push_back(static_cast<float>(rand() % 151));
        }

        // sort them by ascending value
        std::sort(randXVec.begin(), randXVec.end());
        std::sort(randYVec.begin(), randYVec.end());

        // isolate the extreme points
        float minX = randXVec.at(0);
        float maxX = randXVec.at(pointCount - 1);
        float minY = randYVec.at(0);
        float maxY = randYVec.at(pointCount - 1);

        // Divide the interior points into two chains & Extract the vector components
        std::vector<float> xVec;
        std::vector<float> yVec;

        float lastTop = minX, lastBot = minX;

        for (size_t i {1}; i < pointCount - 1; i++) {
            double x = randXVec.at(i);

            if (rand() % 2) {
                xVec.push_back(x - lastTop);
                lastTop = x;
            } else {
                xVec.push_back(lastBot - x);
                lastBot = x;
            }
        }

        xVec.push_back(maxX - lastTop);
        xVec.push_back(lastBot - maxX);

        float lastLeft = minY, lastRight = minY;

        for (size_t i {1}; i < pointCount - 1; i++) {
            double y = randYVec.at(i);

            if (rand() % 2) {
                yVec.push_back(y - lastLeft);
                lastLeft = y;
            } else {
                yVec.push_back(lastRight - y);
                lastRight = y;
            }
        }

        yVec.push_back(maxY - lastLeft);
        yVec.push_back(lastRight - maxY);


        // randomly pair the x and y vector components, we only need to shuffle one of the vectors
        std::random_shuffle(yVec.begin(), yVec.end());

        std::vector<sf::Vector2f> vecs;

        for (size_t i {0}; i < pointCount; i++) {
            vecs.push_back(sf::Vector2f(xVec.at(i), yVec.at(i)));
        }

        // Sort the vectors by angle
        std::sort(vecs.begin(), vecs.end(), [](sf::Vector2f a, sf::Vector2f b){
            return atan2(a.y, a.x) < atan2(b.y, b.x);
        });

        // Lay them end-to-end
        float x = 0, y = 0;
        float minPolygonX = 0;
        float minPolygonY = 0;
        std::vector<sf::Vector2f> points;

        for (int i = 0; i < pointCount; i++) {
            points.push_back(sf::Vector2f(x, y));

            x += vecs.at(i).x;
            y += vecs.at(i).y;

            minPolygonX = std::min(minPolygonX, x);
            minPolygonY = std::min(minPolygonY, y);
        }

        // We end up with a shape that starts at 0,0 and goes up,
        // we need to find the width center point to set the shapes origin. Height is easy as it will just be the height/2
        float xLowest {INFINITY};
        float xHighest {-INFINITY};

        for (size_t i {0}; i < pointCount; i++) {
            float x = points.at(i).x;
            if (x < xLowest) xLowest = x;
            if (x > xHighest) xHighest = x;
        }
        float boundsCenterX = (abs(xLowest) + abs(xHighest))/2.f;
        float centerX = boundsCenterX + xLowest;

        sf::ConvexShape convexShape;
        convexShape.setPointCount(pointCount);

        for (size_t i {0}; i < pointCount; i++) {
            convexShape.setPoint(i, points.at(i));
        }
        

        convexShape.setOrigin({centerX, -convexShape.getGlobalBounds().height/2.f});
        convexShape.setPosition({WINDOW_SIZE.x/2.f, WINDOW_SIZE.y/2.f});
        return convexShape;
    }

public:
    MorphingShape() {
        srand(time(NULL));
        initShapes();
    }

    virtual ~MorphingShape() = default;

    void updateShapePoints() {
        sf::Vector2f point {};
        for (size_t i {0}; i < 8; i++) {
            float t = newShapeTimer/newShapeTimerMax;
            point = lerp(lastShape.getPoint(i), targetShape.getPoint(i), t);
            shape.setPoint(i, point);
        }
    }

    void centerShape() {
        float xLowest {INFINITY};
        float xHighest {-INFINITY};

        for (size_t i {0}; i < 8; i++) {
            float x = shape.getPoint(i).x;
            if (x < xLowest) xLowest = x;
            if (x > xHighest) xHighest = x;
        }
        float boundsCenterX = (abs(xLowest) + abs(xHighest))/2.f;
        float centerX = boundsCenterX + xLowest;

        shape.setOrigin({centerX, -shape.getGlobalBounds().height/2.f});
    }

    void update(float deltaTime) {
        newShapeTimer += deltaTime;
        if (newShapeTimer >= newShapeTimerMax) {
            newShapeTimer = fmod(newShapeTimer, newShapeTimerMax);
            lastShape = targetShape;
            targetShape = generateRandomConvexShape();
        }

        updateShapePoints();
        centerShape();
    }

    void render(sf::RenderTarget& target) {
        target.draw(shape);
    }
};

#endif