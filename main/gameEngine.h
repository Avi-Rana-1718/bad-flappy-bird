#include "entityManager.h"
#include <math.h>
#include <string>

// Constants

#define WINDOW_WIDTH 576
#define WINDOW_HEIGHT 1024

#define WORLD_SX 4
#define WORLD_SY 4

class Assets
{
public:
    std::map<std::string, sf::Texture> m_textures;
    std::map<std::string, sf::Font> m_fonts;
    std::map<std::string, sf::SoundBuffer> m_sounds;

    void addTexture(std::string path)
    {
        sf::Texture texture;
        if (!texture.loadFromFile("assets/sprite/" + path))
        {
            std::cout << path << "not found";
        }
        m_textures[path] = texture;
    }

    void addFont(std::string path)
    {

        sf::Font font;
        if (!font.loadFromFile("assets/" + path))
        {
            std::cout << path << "not found";
        }
        m_fonts[path] = font;
    }
    void addSound(std::string path)
    {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile("assets/audio/" + path))
        {
            std::cout << path << "not found";
        }

        m_sounds[path] = buffer;
    }
};

//

Assets g_assets;
sf::RenderWindow g_window;

int frames;

// Scenes

class Scenes
{
public:
    EntityManager g_entities;
    virtual void init(){

    };
    virtual void run(){

    };

    //
    void sRender(std::vector<Entity *> &entities);

    ~Scenes();
};

Scenes *currentScene = NULL;
void changeScene(Scenes *scene);

void Scenes::sRender(std::vector<Entity *> &entities)
{
    g_window.clear();
    for (auto entity : entities)
    {
        g_window.draw(*(entity->csprite));
        if (entity->ctext != NULL)
        {
            g_window.draw(entity->ctext->text);
        }
    }
    g_window.display();
}

Scenes::~Scenes()
{
    g_entities.clearEntities();
}
// Scene_Over

class Scene_Over : public Scenes
{
public:
    void init();
    void run();
};

void Scene_Over::init()
{
    g_entities.clearEntities();
}

void Scene_Over::run()
{
    sRender(g_entities.getEntities());
}

// Scene_Play

class Scene_Play : public Scenes
{
public:
    Entity *player;

    bool state = 1;

    void init();
    void run();

    float timestamp;

    // Systems
    void sInput();
    void sPhysics(std::vector<Entity *> &entities);
    void spipesHandler(std::vector<Entity *> &entities);
    void sCollision(std::vector<Entity *> &entites);
};

void Scene_Play::init()
{
    g_entities.clearEntities();

    state = 1;

    Entity *bg = g_entities.addEntities("Background", 0.1, 0.1, g_assets.m_textures["background.png"], false);
    bg->csprite->setPosition(0, 0);
    bg->csprite->setScale(sf::Vector2f(2.f, 2.f));
    bg->collision = false;

    player = g_entities.addEntities("Player", 1.f, 10.f, g_assets.m_textures["bird_idle.png"], true);
    player->csprite->setPosition(100, 500);

    Entity *p1 = g_entities.addEntities("Pipe", WORLD_SX, 0.1, g_assets.m_textures["pipe.png"], false);
    p1->csprite->setPosition(300, p1->csprite->getPosition().y);
    Entity *p2 = g_entities.addEntities("Pipe", WORLD_SX, 0.1, g_assets.m_textures["pipe.png"], false);
    p2->csprite->setPosition(650, p2->csprite->getPosition().y);
}

void Scene_Play::run()
{
    sRender(g_entities.getEntities());
    sInput();
    sPhysics(g_entities.getEntities());
    spipesHandler(g_entities.getEntities());
    sCollision(g_entities.getEntities());
}

void Scene_Play::sPhysics(std::vector<Entity *> &entities)
{

    player->csprite->move(0, WORLD_SY);

    for (auto entity : entities)
    {
        if (entity->controllable != true && entity->collision == true)
            entity->csprite->move(-WORLD_SX, 0);
    }
}

void Scene_Play::sInput()
{

    if(frames>timestamp+7)
    {
        player->csprite->setTexture(g_assets.m_textures["bird_idle.png"]);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && (player->csprite->getPosition().y > 0) && state == 1)
    {
        player->csprite->move(0, -player->ctransform->speedY - WORLD_SY);
        player->csprite->setTexture(g_assets.m_textures["bird_up.png"]);
        timestamp=frames;
    }
    else if (state == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    {
        init();
    }
}

void Scene_Play::spipesHandler(std::vector<Entity *> &entities)
{
    for (auto entity : entities)
    {
        if (entity->csprite->getPosition().x + entity->w < 0 && entity->csprite->getScale().y > 0)
        {
            g_entities.removeEntity(entity->e_id);
            Entity *temp = g_entities.addEntities("Pipe", WORLD_SX, 0.1, g_assets.m_textures["pipe.png"], false);
            temp->csprite->setPosition(WINDOW_WIDTH, temp->csprite->getPosition().y);
            Entity *temp2 = g_entities.addEntities("Pipe", WORLD_SX, 0.1, g_assets.m_textures["pipe.png"], false);
            temp2->csprite->setPosition(WINDOW_WIDTH, temp->csprite->getPosition().y - player->h * 4);
            temp2->csprite->setScale(sf::Vector2f(2.f, -2.f));
        }
    }
}

void Scene_Play::sCollision(std::vector<Entity *> &entities)
{
    for (int i = 0; i < entities.size(); i++)
    {
        if (player->csprite->getGlobalBounds().intersects(entities[i]->csprite->getGlobalBounds()) && entities[i]->e_id != player->e_id && entities[i]->collision == true)
        {
            Entity *over = g_entities.addEntities("Game Over", 0.1, 0.1, g_assets.m_textures["gameover.png"], false);
            over->csprite->setPosition((WINDOW_WIDTH - over->w) / 2, (WINDOW_HEIGHT - over->h) / 2);
            over->csprite->setScale(sf::Vector2f(2.f, 2.f));
            over->collision = false;
            state = 0;
        }
    }
}

// Scene_Menu

class Scene_Menu : public Scenes
{
public:
    void init();
    void run();
    void sInput();
};

void Scene_Menu::init()
{
    Entity *bg = g_entities.addEntities("Background", 0.1, 0.1, g_assets.m_textures["background.png"], false);
    bg->csprite->setPosition(0, 0);
    bg->csprite->setScale(sf::Vector2f(2.f, 2.f));
    bg->collision = false;

    Entity *start = g_entities.addEntities("Start", 0.1, 0.1, g_assets.m_textures["start.png"], false);
    start->csprite->setPosition((WINDOW_WIDTH - start->w) / 2, (WINDOW_HEIGHT - start->h) / 2);
    start->csprite->setScale(sf::Vector2f(2.f, 2.f));
    start->collision = false;
}

void Scene_Menu::run()
{
    sRender(g_entities.getEntities());
    sInput();
}

void Scene_Menu::sInput()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        changeScene(new Scene_Play);
        currentScene->init();
    }
}

// GameEngine

class GameEngine
{
    sf::Event events;

    bool is_running;

public:
    GameEngine();
    void run();
};

GameEngine::GameEngine()
{
    is_running = true;
    frames = 0;
    changeScene(new Scene_Menu);

    g_window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Bad FlappyBird");
    g_window.setFramerateLimit(60);

    g_assets.addTexture("bird_idle.png");
    g_assets.addTexture("bird_down.png");
    g_assets.addTexture("bird_up.png");
    g_assets.addTexture("pipe.png");

    g_assets.addTexture("background.png");
    g_assets.addTexture("start.png");
    g_assets.addTexture("gameover.png");

    g_assets.addFont("noto.ttf");

    g_assets.addSound("wing.wav");
}

void GameEngine::run()
{
    currentScene->init();
    while (is_running)
    {
        currentScene->run();
        while (g_window.pollEvent(events))
        {
            if (events.type == sf::Event::Closed)
            {
                g_window.close();
                is_running = false;
            }
        }

        frames++;
    }
}

void changeScene(Scenes *scene)
{
    delete currentScene;
    currentScene = scene;
}