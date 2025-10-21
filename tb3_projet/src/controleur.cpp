#include "geometry_msgs/Vector3.h"
#include "ros/duration.h"
#include "ros/node_handle.h"
#include <csignal>
#include <geometry_msgs/Twist.h>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>
#include "std_msgs/Int8.h"

// a completer

#define FREQUENCY 1
#define MAX_LINEAR_SPEED 0.15
#define MAX_ANGULAR_SPEED 2.84
#define LINEAR_SPEED 0.1
#define ANGULAR_SPEED 0.1

using namespace ros;
using namespace std_msgs;
using namespace geometry_msgs;

// Les prototypes des fonctions
void sigintHandler(int sig);

// Les variables globales
Publisher pub_msg;
Publisher pub_avancer;

Subscriber sub_key;

String msg;
double linear_speed = LINEAR_SPEED;
double angular_speed = ANGULAR_SPEED;
double frequence = FREQUENCY;
geometry_msgs::Twist ROBOT;
double freq;
int mode = 0;
bool Auto = true;

int num_key;




// L'impl des fonctions
void arreter() {
  ROBOT.linear.x = 0.0;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = 0.0;
  pub_avancer.publish(ROBOT);
}

void avancer(double v) {
  ROBOT.linear.x = v;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = 0.0;
  pub_avancer.publish(ROBOT);
}

void send_msg(String msg) {

  ROS_INFO("Publisher : %s", msg.data.c_str());
  // a completer...
}

void reculer(double v) {
  ROBOT.linear.x = -v;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = 0.0;
  pub_avancer.publish(ROBOT);
}

void move(int mode) {
  ROS_INFO("move");
  
  switch (mode){
  
    case(0) : 
    avancer(0.3); 
    break;
    case(1) :
    arreter(); 
    break;
    case(2) :
    reculer(0.3); 
    break;

  }

}


void tourner_d(double a) {
  ROBOT.linear.x = 0.0;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = -a;
  pub_avancer.publish(ROBOT);
}


void tourner_g(double a) {
  ROBOT.linear.x = 0.0;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = a;
  pub_avancer.publish(ROBOT);
} 



void keyboardCallback(Int8 keyboard_input){
  ROS_INFO("Keyboard listener : I heard : [%d]", keyboard_input.data);
  num_key = keyboard_input.data;
}




int main(int argc, char **argv) {

  signal(SIGINT, sigintHandler);
  ros::init(argc, argv, "controleur", ros::init_options::NoSigintHandler);
  ros::init(argc, argv, "keyboard", ros::init_options::NoSigintHandler);

  NodeHandle nh;
  nh.getParam("freq", freq);

  //Publishers
  pub_msg = nh.advertise<String>("/log", 2); // Le publisher
  pub_avancer = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10); // Le publisher
      
  //Subscribers
  sub_key = nh.subscribe("/keyboard_input", 1000, keyboardCallback);

  Rate loop_rate(frequence);

  while (ok()) {


          
          if (num_key == 97){
            Auto = !Auto;
            
            ROS_INFO("Auto = %d", Auto);
            
          }
      
      
          if (Auto){
                ROS_INFO("J'AI UNE CONSCIENCE");
                ROS_INFO("Mode : %d", mode);
                
                move(mode);
                mode++;
          
                if (mode > 2){
                  mode = 0;
                }
          
          } 
            
          else {
              ROS_INFO("APPUYEZ SUR ZQSD");
              switch (num_key){
        
                case(122) : 
                avancer(0.5); 
                ROS_INFO("J'AVANCE");
                break;
                case(115) :
                reculer(0.5); 
                ROS_INFO("JE RECULE");
                break;
                case(113) :
                tourner_g(1); 
                ROS_INFO("JE TOURNE A GAUCHE");
                break;
                case(100) :
                tourner_d(1); 
                ROS_INFO("JE TOURNE A DROITE");
                break;
      
              }
              
        num_key = 32; //Valeur arbitraire pour ne pas garder la même valeur pour num_key (mettre une valeur inutilisée)
              
      }

    spinOnce();
    loop_rate.sleep();
  }
  return 0;
}

/**
 *
 * SIGINT handler.
 * Exit code gracefully.
 *
 * @param sig - catched signal.
 *
 */

void sigintHandler(int sig) {
  // Log quit
  ROS_INFO("Exiting program gracefully ...");

  arreter();
  // MESSAGE A PUBLIER le MESSAGE

  // Kill all open subscriptions, publications, service calls, and service
  // servers
  shutdown();
}
