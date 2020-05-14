//Evgenia Koreslkaya
//Space game using sfml to feel relax in space

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <SFML/Window.hpp> 
#include <SFML/System.hpp>
#include <cstdlib>
#include <ctime> 
#include <cmath>
#include <sstream> 
#include <list>
#include <vector> 
#include <utility>
#include <iomanip>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "point.h"

using namespace std;
using namespace sf;

//////////////global constants/////////////////////
const int H = 877;
const int W = 1200;
int score = 0;
float g = 6.67408 / pow(10, 11);
float DEGTORAD = 0.0174532925f;

/////////////////all classes and functions///////////////////
class Animation; //creating animation for objects
class Entity; //base class for planets and ship
class player; //ship
class Planet; //planets, Sun and black hole
class Car; //car, which can ride on planets 

Point refine(Point p1, Point p2, int r1, int r2); //doesn't allow intersection
bool isNear(Point p1, Point p2, int r1, int r2, float k);//check relative postion


///////////////////////////definitions/////////////////////////

class Animation
{
public:
    float Frame, speed;
    Sprite sprite;
    vector<IntRect> frames;

    Animation(){}

    Animation (Texture &t, int x, int y, int w, int h, int count, float Speed)
        {
             	Frame = 0;
        	speed = Speed;

        	for (int i=0;i<count;i++)
         		frames.push_back(IntRect(x+i*w, y, w, h));

                sprite.setTexture(t);
                sprite.setOrigin(w/2,h/2);
        	sprite.setTextureRect(frames[0]);
        }


        void update()
        {
    	        Frame += speed;
                int n = frames.size();
                if (Frame >= n) Frame -= n;
                if (n>0) sprite.setTextureRect( frames[int(Frame)] );
        }

        bool isEnd()
        {
          return Frame+speed>=frames.size();
        }
};

class Entity
{
public:
	float x_global = 0;
	float y_global = 0;
	float x,y,dx,dy,R,angle;
	bool life;
	string name;
	Animation anim;
	int density;
	float m;
	float acceleration = 0.05;
Entity()
{
  life=1;
}

void settings(Animation &a,float X,float Y,float Angle=0,int radius=1,int _density=0)
{
  anim = a;
  x=X; y=Y;
  angle = Angle;
  R = radius;
  x_global = X;
  y_global = Y;
  density = _density;
  m = (4 * 3.14 * pow(R, 3) * density) / 3;

}

virtual void update(){};

virtual void Move(float dx, float dy) {};

void draw(RenderWindow &app)
{
  anim.sprite.setPosition(x,y);
  anim.sprite.setRotation(angle+90);
  app.draw(anim.sprite);

  CircleShape circle(R);
  circle.setFillColor(Color(255,0,0,170));
  circle.setPosition(x,y);
  circle.setOrigin(R,R);
}

virtual ~Entity(){};
};

class Car : public Entity {
public:
	bool right = false;
       	bool left = false;
	bool ride = false;	

	Car() {
	    name = "car";
	} 
	Point update_car(float planet_r) {
	    Point result;
	    float alp = 0;
	    
	    if(right) 
                alp = 1 * 128 / planet_r;
	    else if(left)
		alp = -1 * 128 / planet_r;

	    if(alp != 0) {
	     result.y = (planet_r + R) * (sin(DEGTORAD*fmod((angle + alp),360)) - 
			                  sin(DEGTORAD*fmod(angle,360)));
             result.x = (planet_r + R) * (cos(DEGTORAD*fmod((angle + alp),360)) - 
			                  cos(DEGTORAD*fmod(angle,360)));
	 
	     angle += alp;
	     
	    }

	    return result;
	} 
};

class player: public Entity
{
public:
   bool thrust;
   float fuel = 1000;
   
   bool onplanet = false;
   bool iscar = false; 
   float maxSpeed = 15.f;
   float health = 1000.f;

   Car* car = new Car(); 
   Entity* planet = new Entity();
   
   player()
   {
     name="player";
   }
   Point getDelta(list<Entity*> p) {
	   Point ref;
	   onplanet = false;
	   for(auto it : p) {
		   if(isNear(Point(x,y),Point(it->x,it->y),R,it->R,1000)) {
			float r_2 = pow((x - it->x), 2) + pow((y - it->y), 2);
			float a = g * it->m / r_2;
	 		if(!iscar) {
			 dx += a * (it->x - x) / sqrt(r_2);
			 dy += a * (it->y - y) / sqrt(r_2);
			}
                        if (isNear(Point(x,y),
                                   Point(it->x - dx,
                                         it->y - dy),
                                   R,it->R,1.01)) {
                            onplanet = true;
                            planet = it;
			}
			if(!iscar) {
			 if (isNear(Point(x,y),
				   Point(it->x - dx,
					 it->y - dy),
				   R,it->R,1)) {
			    ref = refine(Point(x,y), 
			    		 Point(it->x - dx, it->y - dy), R,it->R);
			    if(sqrt(pow(dx + ref.x, 2) + pow(dy + ref.y, 2)) > 1)
				    health = health - 1
					              *sqrt(pow(dx + ref.x, 2)+                                                               pow(dy + ref.y,2));
			    if(!thrust) {
				dx *= 0.95;
				dy *= 0.95; 
			    }
			 } 
			}			
		   }

	   }
	   if(!onplanet) 
		   iscar = false;
	   dx += ref.x;
	   dy += ref.y;
	   return Point(dx, dy);
   }

   void update() { 
     if(!iscar) {
      if (thrust) {

	dx+=cos(angle*DEGTORAD) * acceleration;
        dy+=sin(angle*DEGTORAD) * acceleration; 
      }
      else { 
	dx*=0.99;
        dy*=0.99; 
      }

     float speed = sqrt(dx*dx+dy*dy);

     if (speed>maxSpeed) {	    
        dx *= maxSpeed/speed;
        dy *= maxSpeed/speed; 
      }
   }  
   else {
       Point delta = car->update_car(planet->R);  
       anim.sprite.move(-delta.x, -delta.y);
       x -= delta.x;
       y -= delta.y; 
       dx = delta.x;
       dy = delta.y;
   } 
 }
   float getSpeed(){
       return sqrt(dx*dx+dy*dy);
   }

};

Point refine(Point p1, Point p2, int r1, int r2) {
        Point res;
        float k = (r1 + r2) / (sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)));
        res.x = (p1.x - p2.x) * k;
        res.y = (p1.y - p2.y) * k;
	return Point(res.x - p1.x + p2.x, res.y - p1.y + p2.y);

}
 
bool BelongToCircle(Point p, Point center, double r) {
	return sqrt(pow(p.x - center.x, 2) + pow(p.y - center.y, 2)) <= r;
}

class Planet : public Entity {

public:
        Planet() {
	    dx = 0;
	    dy = 0;
	    name = "planet";

        }
        
        void Move(float dx, float dy) {
                anim.sprite.move(dx, dy);
                x += dx;
                y += dy;
        }
	Point getPosition() {
		return Point(x_global, y_global);
	}

};


bool isNear(Point p1, Point p2, int r1, int r2, float k) {
	return pow((p1.x - p2.x), 2) + 
	       pow((p1.y - p2.y), 2) < 
	       pow(k*(r1 + r2), 2);
}

int main()
{ 
    srand(time(0));
    unsigned long long cadr_number = 0;
    string can_drive = "No car";
    float alpha = 0;
    float player_angle = 0; 

    RenderWindow window(VideoMode(W, H), "WONDERFUL SPACE");
    window.setFramerateLimit(60); 
    
    ////////////download font and text style//////////////////////////   
    Font font, font1;
    font.loadFromFile("calibri.ttf");
    font1.loadFromFile("Beautiful Heart.ttf");
    Text text("", font1, 30);
    text.setFillColor(Color::Red);
    text.setPosition(5.f, 5.f); 
    
    Text text1("", font, 15); 
    text1.setFillColor(Color::Green);
    text1.setPosition(5.f, 200.f);
    
    Text text2("", font, 25);
    text2.setFillColor(Color::Blue);
    text2.setPosition(450.f, 5.f);

    Text text3("", font1, 100);
    text3.setFillColor(Color::White);
    text3.setPosition(270.f, 450.f);

    //////////////playing background music///////////////////////////
    SoundBuffer buffer;
    if (!buffer.loadFromFile("Space.wav"))
        return -1;
    Sound sound;
    sound.setBuffer(buffer);
    sound.setLoop(true);
    sound.play();
    
    //////////////sound of rocket in turbo mode////////////////////
    SoundBuffer buf_rocket;
    if (!buf_rocket.loadFromFile("rocket.wav"))
        return -1;
    Sound rocket;
    rocket.setBuffer(buf_rocket);
    
    ////////////draw pictures and create animation///////
    Texture t1, t2, t3, t4, t5;
    t1.loadFromFile("planets/spaceship(1).png");
    t2.loadFromFile("space.png");
    t3.loadFromFile("planets/sun2048.png");
    t4.loadFromFile("car40.png");
    t5.loadFromFile("Hole.png");

    string dir = "planets/"; 

    vector<string> values = {"128.png", "256.png", "512.png"};

    vector<string> pictures_names = { "io", "mars", "neptun", "sun" };
    
    vector<Texture> textures(pictures_names.size() * 3);
    
    for(int i = 0; i < textures.size(); ++i) {
	    textures[i].loadFromFile(dir + 
		        pictures_names[i % pictures_names.size()] + 
			values[i / pictures_names.size()]);
    }

    t1.setSmooth(true);
    t2.setSmooth(true);
    t3.setSmooth(true);
    t4.setSmooth(true);
    t5.setSmooth(true);

    for(auto& it : textures) 
	    it.setSmooth(true);


    Sprite background(t2);

    Animation sPlayer(t1, 40,0,40,40, 1, 0);
    Animation sPlayer_ac(t1, 38,40,44,45, 1, 0);
    Animation sPlayer_go(t1, 40,85,40,40, 1, 0);
    Animation sSun(t3, 0, 0, 2048, 2048, 1, 0);
    Animation sCar(t4, 0, 0, 40, 40, 1, 0);
    Animation sHole(t5, 0, 0, 1230, 1264, 1, 0);

    list<Entity*> planets;
    int k = 0;
    while (k < 18 * textures.size()) {
	int number = rand()%textures.size();
	int x1 = rand()%12000;
	int y1 = rand()%12000;
	int size = 0;
	if (number < textures.size() / 3) 
		size = 128;
	else if((number >= textures.size() / 3) && (number < 2*textures.size()/3)) 		  size = 256;	
	else  
		size = 512;

	bool indic = false;
	for(auto it : planets) {
	  if(isNear(Point(x1, y1), Point(it->x, it->y), size/2,it->R, 3)) {
			indic = true;
			break;
		} 
	} 
	if(indic) 
		continue;

	Animation sPlanet(textures[number], 0, 0, size, size, 1, 0);
	Planet* pl = new Planet();
	pl->settings(sPlanet,x1,y1,rand()%360, size/2, 400000);
	planets.push_back(pl);
	++k;
    }
    Planet* pl = new Planet();
    pl->settings(sSun,rand()%10000 + 12000, rand()%10000 + 10000,rand()%360,
		 1024, 400000);
    planets.push_back(pl);

    Planet* bh = new Planet();
    bh->settings(sHole,-(rand()%10000 + 12000), -(rand()%10000 + 10000),
		 rand()%360, 100, 90000000);
    planets.push_back(bh);

    player *p = new player();
    p->settings(sPlayer,static_cast<int>(W/2),static_cast<int>(H/2),0,20,2700);

    //////////////special for coordinates;)////////////////////////////
    float pos_x = p->x;
    float pos_y = p->y;

    while (window.isOpen())
    {	
	++cadr_number;

	Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (Keyboard::isKeyPressed(Keyboard::Escape))  
		return false; 
	    
	}

	(*prev(planets.end()))->angle += 1;

	if(!p->iscar) {
         if (Keyboard::isKeyPressed(Keyboard::LShift) && p->thrust) {
		if(p->fuel > 0)
		    p->maxSpeed = 30;
		else if (p->maxSpeed > 15)
			p->maxSpeed *= 0.99;

		if(p->fuel > 0) {
			rocket.play();
			p->acceleration += 0.01;
			p->fuel -= 10;
		}
		else {
			p->acceleration = 0.05;
		}
	 } 

	 else { 
		p->acceleration = 0.05; 
		
		if(p->maxSpeed > 15)
			p->maxSpeed *= 0.99;

		if(p->fuel < 1000)
			p->fuel += 10;
	 } 

	
	 if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle+=3;
         if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle-=3;
        
	 if (Keyboard::isKeyPressed(Keyboard::Up)) {
	       	p->thrust=true;
         }
         else p->thrust=false;
	
         alpha = acos((p->planet->y - p->y) / 
	              (sqrt(pow(p->x - p->planet->x, 2) +
                       pow(p->y - p->planet->y, 2)))) /
                 DEGTORAD;
         if(p->x < p->planet->x) {
         	alpha = -alpha;
         }
	 player_angle = fmod(p->angle + 90, 360);

	 if(player_angle > 180) {
         	player_angle = player_angle - 360;
         }

	 if (p->onplanet) {
          if(abs(player_angle - alpha) < 30) {
		 can_drive = "Press space to turn into the car!";
	  }
	  else {
		 can_drive = "You are on planet, land the rocket!";
	  }
	 }

	 else {
		 can_drive = "Flying! Land on a planet to drive!\n";
	         can_drive += "Press 'LShift' for turbo mode!";
	 }
	 if(Keyboard::isKeyPressed(Keyboard::Space)) {
		if(p->onplanet && cadr_number > 20) {

			if(abs(player_angle -  alpha) < 30) {
			 cadr_number = 0;
			 p->iscar = true;
			 p->car->settings(sCar, p->x, p->y, alpha - 90, 20, 2000);
			}	
		}
	 }
								

    	 if (p->thrust) {
		if(abs(p->acceleration - 0.05) < 0.001) 
	      		p->anim = sPlayer_go;
		else 
			p->anim = sPlayer_ac;
	 }
         else   p->anim = sPlayer;
	}
	
	else {
            if(p->health < 1200)
                  p->health += 0.1 * p->getSpeed();

	 if(Keyboard::isKeyPressed(Keyboard::Space)) {
		 if(cadr_number > 20) {
			if(isNear(Point(p->x, p->y), Point(p->car->x, p->car->y),
			          p->R, p->car->R, 1)) {
		 	p->iscar = false;
			Point delta(p->x - p->car->x, p->y - p->car->y);
			
			p->anim.sprite.move(-delta.x, -delta.y);
			p->x += -delta.x;
			p->y += -delta.y;

			for(auto it : planets) {
				it->Move(-delta.x, -delta.y);
			}	
			cadr_number = 0; 
			}
		 }
	 }
	 if (Keyboard::isKeyPressed(Keyboard::Right)) {
		 p->car->right = true;
		 p->car->left = false;
	 }
	 else if (Keyboard::isKeyPressed(Keyboard::Left)) { 
		 p->car->left = true;
		 p->car->right = false;
	 } 
	 else {
                 p->car->left = false;
                 p->car->right = false;
	 }
	}
	
	p->update();
	p->anim.update();
	Point delta = p->getDelta(planets);

        p->car->anim.update();

	for (auto& it : planets) 
		it->Move(-delta.x, -delta.y);

        window.clear();
	window.draw(background);

	//////////////////////////draw objects//////////////////////////////

	ostringstream Speed;
	ostringstream Acceleration;
        ostringstream playerPosString;
	ostringstream globalcoords;
        ostringstream Fuel;
	ostringstream Health;

	pos_x += delta.x;
	pos_y += delta.y;
	playerPosString << static_cast<int>(pos_x) << " "
                               << static_cast<int>(pos_y);
        
	Speed << fixed << setprecision(2) << p->getSpeed(); 
	Acceleration << fixed << setprecision(2) << p->acceleration;
	
	globalcoords << endl;
	
	auto it_last = prev(planets.end());

	globalcoords<<"Black Hole: "<< (*it_last)->x_global << " " <<
		                   (*it_last)->y_global << " " <<
				   /*(*it_last)->R <<*/ endl;
        globalcoords << "Sun: "<< (*prev(it_last))->x_global << " " <<
                                   (*prev(it_last))->y_global << " " <<
                                   /*(*prev(it_last))->R <<*/ endl;

	Fuel << p->fuel;
	Health << static_cast<int>(p->health);

	/*for(auto it : planets) {
		globalcoords << it->x_global << " " << it->y_global << " " <<
		         	it->R <<"\n";
	}*/

        text.setString("Position: " + playerPosString.str() + "\n" +
                       "Speed: " + Speed.str() + "\n" +
		       "Acceleration: " + Acceleration.str() +"\n" + 
		       "Fuel: " + Fuel.str() + "\n" +
		       "Health: " + Health.str() + "\n");

	text1.setString(/*"Planets: " + */globalcoords.str());
	
	if(p->iscar)  {
		if(isNear(Point(p->x, p->y), Point(p->car->x, p->car->y),
                                  p->R, p->car->R, 1)) 
			can_drive = "Press space to turn into the rocket!";
		else
			can_drive = "Moving! Return to the rocket for flight!";
	}

	text2.setString(can_drive);

	for (auto it : planets)
                it->draw(window);

        p->draw(window);

        if(p->iscar)
                p->car->draw(window);

        window.draw(text);
	window.draw(text1);
	window.draw(text2);

	if(p->health < 0) {
	     text3.setString("GAME OVER!\nPress space to escape");
	     window.draw(text3);
	}
	window.display();

	if(p->health < 0) {
             this_thread::sleep_for(chrono::milliseconds(2000));
	     while(true) {
	         if(Keyboard::isKeyPressed(Keyboard::Space))
			 break;
	     }
             window.close();
	}
    }
    return 0;
}

