#ifndef _HELPER_HPP_
#define _HELPER_HPP_

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

sf::Vector2f lerp(sf::Vector2f a, sf::Vector2f b, float t){
  return a*(1-t) + b*t;
}

#endif