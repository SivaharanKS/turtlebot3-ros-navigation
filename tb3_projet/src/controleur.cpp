#include "geometry_msgs/Vector3.h"
#include "ros/duration.h"
#include "ros/node_handle.h"
#include <csignal>
#include <geometry_msgs/Twist.h>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>

#include "ros/ros.h"
#include "std_msgs/Int8.h"
#include "termios.h"
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
String msg;
double linear_speed = LINEAR_SPEED;
double angular_speed = ANGULAR_SPEED;
double frequence = FREQUENCY;
geometry_msgs::Twist ROBOT;
double freq;
int mode = 0;
bool Auto = true;

int getch();
std_msgs::Int8 keyboard_input;
Publisher KeyInput_Pub;


// L'impl des fonctions
void arreter() {
  pub_avancer.publish(ROBOT);
  ROBOT.linear.x = 0.0;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
}

void avancer(double v) {
  pub_avancer.publish(ROBOT);
  ROBOT.linear.x = v;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
}

void send_msg(String msg) {

  ROS_INFO("Publisher : %s", msg.data.c_str());
  // a completer...
}

void reculer(double v) {
  pub_avancer.publish(ROBOT);
  ROBOT.linear.x = -v;
  ROBOT.linear.y = 0.0;
  ROBOT.linear.z = 0.0;
}

void move() {
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


void tourner_d() {
  pub_avancer.publish(ROBOT);
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = 10.0;
}


void tourner_g() {
  pub_avancer.publish(ROBOT);
  ROBOT.angular.x = 0.0;
  ROBOT.angular.y = 0.0;
  ROBOT.angular.z = -10.0;
}

int main(int argc, char **argv) {

  signal(SIGINT, sigintHandler);
  ros::init(argc, argv, "controleur", ros::init_options::NoSigintHandler);

  NodeHandle nh;
  nh.getParam("freq", freq);

  //Publishers
  pub_msg = nh.advertise<String>("/log", 2); // Le publisher
  pub_avancer =
      nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10); // Le publisher
      

  KeyInput_Pub = nh.advertise<std_msgs::Int8>("/keyboard_input", 1000);

  Rate loop_rate(frequence);

  while (ok()) {
    keyboard_input.data = getch();
    KeyInput_Pub.publish(keyboard_input);
    ROS_INFO("keyb = %d", keyboard_input.data);
    ros::spinOnce();
    loop_rate.sleep();

    if (keyboard_input.data == 97){
      Auto = !Auto;
      
      ROS_INFO("Auto = %d/n", Auto);
      
    }


    if (Auto){
          ROS_INFO("J'AI UNE CONSCIENCE");
      
          move();
          mode++;
    
          if (mode > 2){
            mode = 0;
          }
    
    } 
      
    else {
        ROS_INFO("APPUYEZ SUR ZQSD");
        switch (keyboard_input.data){
  
          case(122) : 
          avancer(0.5); 
          break;
          case(115) :
          reculer(0.5); 
          break;
          case(113) :
          tourner_g(); 
          break;
          case(100) :
          tourner_d(); 
          break;

        }

    
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
int getch() {
  static struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt); // save old settings
  newt = oldt;
  newt.c_lflag &= ~(ICANON);               // disable buffering
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // apply new settings

  int c = getchar(); // read character (non-blocking)

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
  return c;
}  

void sigintHandler(int sig) {
  // Log quit
  ROS_INFO("Exiting program gracefully ...");

  arreter();
  // MESSAGE A PUBLIER le MESSAGE

  // Kill all open subscriptions, publications, service calls, and service
  // servers
  shutdown();
}
