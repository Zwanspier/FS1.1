#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;
using namespace std;

int main()
{
    RenderWindow window(VideoMode({ 1280, 1024 }), "The Menu Game");
    CircleShape shape(20.f);
    shape.setFillColor(Color::Blue);
    shape.setPosition({ 590, 512 });

    // Open a font  
    Font font("arial.ttf");

    // Create a text
    Text text(font, "The Menu Game", 100);
    text.setStyle(Text::Italic | Text::Bold);
    text.setFillColor(Color::Red);
    auto textbounds = text.getLocalBounds();
    text.setOrigin(Vector2f(textbounds.size.x / 2.0f, textbounds.size.y / 2.0f));
    text.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 4.f));

    Text text2(font, "1. Start\n2. Settings\n3. Exit", 50);
    text2.setStyle(Text::Italic | Text::Bold);
    text2.setFillColor(Color::Red);
    auto text2bounds = text2.getLocalBounds();
    text2.setOrigin(Vector2f(text2bounds.size.x / 2.0f, text2bounds.size.y / 2.0f));
    text2.setPosition(Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.draw(text);
        window.draw(text2);
        window.display();
    }
}