#include "geometry_msgs/Vector3.h"
#include "ros/duration.h"
#include "ros/node_handle.h"
#include <csignal>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/Quaternion.h>
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>
#include "std_msgs/Int8.h"
#include <tf/tf.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Empty.h>
#include <cmath>

//define

#define FREQUENCY 5
#define MAX_LINEAR_SPEED 0.15
#define MAX_ANGULAR_SPEED 2.84
#define LINEAR_SPEED 0.5
#define ANGULAR_SPEED 1
#define PI 3.14159265359 // Valeur de pi

//namespace

using namespace ros;
using namespace std_msgs;
using namespace geometry_msgs;

// Les variables globales
Publisher pub_msg;
Publisher pub_avancer;
Publisher pub_reset;

Subscriber sub_key;
Subscriber sub_pose;

String msg;
double linear_speed = LINEAR_SPEED;
double angular_speed = ANGULAR_SPEED;
double frequence = FREQUENCY;
double freq;

Twist ROBOT; // Variable contenant deux vecteurs

int mode = 0; // Variable mode utiliser dans la fonction move
bool Auto = true; // Variable pour contenir le mode automatique
int num_key; // variable pour stocker les code liée au lettre du clavier

Quaternion q_angle;

//Variables de position
double pos_x = 0.0; // Position sur l'axe x
double pos_y = 0.0; // Position sur l'axe y
double pos_ang = 0.0; // Position angulaire sur l'axe z

// Les prototypes des fonctions
void sigintHandler(int sig);

// L'impl des fonctions

/* Fonction arreter(), arrêt le movement du robot en mettant
 * les vitesses linéaires et angulaire à zéro, puis on le publie
 * sur le topic /cmd_vel avec le publisher pub_avancer
 */
void arreter() {
    ROBOT.linear.x = 0.0;
    ROBOT.linear.y = 0.0;
    ROBOT.linear.z = 0.0;
    ROBOT.angular.x = 0.0;
    ROBOT.angular.y = 0.0;
    ROBOT.angular.z = 0.0;
    pub_avancer.publish(ROBOT);
}

/* Fonction avancer(), fait avancer le robot sur l'axe x
 * on mettant une valeur dans v, qui sera la vitesse linéaire
 * et on met 0 au cas ou sur les autres axes mais qui n'est pas
 * necessaire, puis on le publie sur le topic /cmd_vel avec
 * le publisher pub_avancer
 */
void avancer(double v) {
    v = std::max(-MAX_LINEAR_SPEED, std::min(MAX_LINEAR_SPEED, v)); // saturation
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

/* Fonction reculer(), fait reculer le robot sur l'axe x
 * on mettant une valeur dans v (on le met on négative pour le
 * faire reculer), qui sera la vitesse linéaire
 * et on met 0 au cas ou sur les autres axes mais qui n'est pas
 * necessaire, puis on le publie sur le topic /cmd_vel avec
 * le publisher pub_avancer
 */
void reculer(double v) {
    ROBOT.linear.x = -v;
    ROBOT.linear.y = 0.0;
    ROBOT.linear.z = 0.0;
    ROBOT.angular.x = 0.0;
    ROBOT.angular.y = 0.0;
    ROBOT.angular.z = 0.0;
    pub_avancer.publish(ROBOT);
}

/* Fonction move(), fait appel à un switch qui passe d'un
 * d'un mode à l'autre, cette fonction permet de faire des
 * actions de manière autonome en utilisant les fonctions
 * avancer(), reculer() et arreter()
 */
void move(int mode) {
    ROS_INFO("move");

    switch (mode) {
    case(0):
        avancer(0.3);
        break;
    case(1):
        arreter();
        break;
    case(2):
        reculer(0.3);
        break;
    }
}

/* Fonction tourner_d(), permet de tourner à droite, on
 * on procède de la même manière que pour reculer(), sauf
 * qu'on met une valeur pour angular z (avec une valeur négative)
 */
void tourner_d(double a) {
    a = std::max(-MAX_ANGULAR_SPEED, std::min(MAX_ANGULAR_SPEED, a)); // saturation
    ROBOT.linear.x = 0.0;
    ROBOT.linear.y = 0.0;
    ROBOT.linear.z = 0.0;
    ROBOT.angular.x = 0.0;
    ROBOT.angular.y = 0.0;
    ROBOT.angular.z = -a;
    pub_avancer.publish(ROBOT);
}

/* Fonction tourner_g(), permet de tourner à gauche, on
 * on procède de la même manière que pour avancer(), sauf
 * qu'on met une valeur pour angular z (avec une valeur positive)
 */
void tourner_g(double a) {
    a = std::max(-MAX_ANGULAR_SPEED, std::min(MAX_ANGULAR_SPEED, a)); // saturation
    ROBOT.linear.x = 0.0;
    ROBOT.linear.y = 0.0;
    ROBOT.linear.z = 0.0;
    ROBOT.angular.x = 0.0;
    ROBOT.angular.y = 0.0;
    ROBOT.angular.z = a;
    pub_avancer.publish(ROBOT);
}

/* Fonction de rappel, keyboardCallback(), utilisant un subscriber recu du topic /keyboard_input
 * et affiche la valeur associer au lettre appuyer puis le mémorise dans une
 * variable.
 */
void keyboardCallback(Int8 keyboard_input) {
    ROS_INFO("Keyboard listener : I heard : [%d]", keyboard_input.data);
    num_key = keyboard_input.data;
}

/* Fonction de rappel, PoseCallback(), qui utilise /Odom,
 * récupére les positions et les affiches
 */
void PoseCallback(const nav_msgs::Odometry::ConstPtr& msg) {
    pos_x = msg->pose.pose.position.x; // récupére la position linéaire sur l'axe x
    pos_y = msg->pose.pose.position.y; // récupére la position linéaire sur l'axe y
    q_angle = msg->pose.pose.orientation; // récupére l'orientation du robot avec quaternion 

    double yaw = tf::getYaw(q_angle); // récupére l'angle sur l'axe z en radian (quaternion -> radian)
    pos_ang = yaw * 180.0 / PI;    // conversion en degrés (radian -> degré)

    //ROS_INFO("Position : x = %f m; y = %f m; theta = %f deg", pos_x, pos_y, pos_ang);
    // affiche les position sur axe x, axe y et l'angle z
}

/* Fonction avancer_Continue(), permet dans donner une distance
 * et le robot doit effectuer cette distance
 */
void avancer_Continue(double distance_voulue) {
    double position_depart = pos_x; // position départ du robot
    double distance_parc = 0.0;  // la distance parcourut par le robot
    double y_depart = pos_y;
    ros::Rate rate(10); // défini un cycle qui se répéte 10 fois par secondes

    while (distance_parc < (distance_voulue * 0.98)) { // (+/- 0.02 au maximum) 
        spinOnce(); // mise à jour des données odom
        distance_parc = sqrt(pow(pos_x - position_depart, 2) + pow(pos_y - y_depart, 2));; // distance parcourut

        // Proportionnel 
        double Kp = 3.5;
        double erreur_p = distance_voulue - distance_parc;

        // Integrateur
        double Ki = 3.5;
        double erreur_i = erreur_p * (1 / freq);

        // Derivateur
        //Kd = 1.0;
        //double erreur_d = 

        double vitesse = (Kp * erreur_p) + (Ki * erreur_i); // gain proportionnel avec Kp = 1.2

        avancer(vitesse);
        // ROS_INFO("Distance parcourue : %f / %f", distance_parc, distance_voulue);
        rate.sleep(); // une pause 
    }

}
/* Fonction cal_delta_Theta(), prend deux variables en entrée
 * et calcul l'écart entre deux position angulaire
 */
float cal_delta_Theta(float _theta, float _theta0) {

    float theta, theta0, delta_theta;
    if (_theta < 0)  theta = _theta + 2 * M_PI;
    else theta = _theta;

    if (_theta0 < 0)  theta0 = _theta0 + 2 * M_PI;
    else theta0 = _theta0;

    delta_theta = theta - theta0;

    if (delta_theta < -M_PI) {
        delta_theta = delta_theta + 2 * M_PI;
    }
    else {
        if (delta_theta >= M_PI) {
            delta_theta = delta_theta - 2 * M_PI;
        }
    }
    return delta_theta;
}

/* Fonction rotation_Continue(), permet dans donner un angle
 * et le robot doit effectuer cette rotation
 */
void rotation_Continue(double angle_voulue_deg) { // récupére un angle en degré

    double angle_voulue_rad = angle_voulue_deg * PI / 180.0; // degré -> radian  
    double angle_depart = pos_ang; // récup l'angle de départ
    double angle_parc = 0.0; // récup l'angle parcourut
    ros::Rate rate(10); // défini un cycle qui se répéte 10 fois par secondes

    if (angle_voulue_deg < 0) {
        while (angle_parc > angle_voulue_deg * 0.95) { // (+/- 0.05 au maximum) 
            spinOnce(); // mise à jour des données odom
            angle_parc = pos_ang + angle_depart; // angle parcourut

            double erreur = cal_delta_Theta(angle_voulue_rad, angle_parc * PI / 180.0);
            double vitesse = 1.2 * erreur; // gain proportionnel avec Kp = 1.2

            ROBOT.linear.x = 0.0;
            ROBOT.linear.y = 0.0;
            ROBOT.linear.z = 0.0;
            ROBOT.angular.x = 0.0;
            ROBOT.angular.y = 0.0;
            ROBOT.angular.z = std::max(-MAX_ANGULAR_SPEED, std::min(MAX_ANGULAR_SPEED, vitesse)); // saturation
            pub_avancer.publish(ROBOT);
            rate.sleep(); // une pause 
        }
    }

    else {
        while (angle_parc < angle_voulue_deg * 0.95) { // (+/- 0.05 au maximum) 
            spinOnce(); // mise à jour des données odom
            angle_parc = fabs(pos_ang - angle_depart); // angle parcourut

            double erreur = cal_delta_Theta(angle_voulue_rad, angle_parc * PI / 180.0);
            double vitesse = 1.2 * erreur; // gain proportionnel avec Kp = 1.2

            ROBOT.linear.x = 0.0;
            ROBOT.linear.y = 0.0;
            ROBOT.linear.z = 0.0;
            ROBOT.angular.x = 0.0;
            ROBOT.angular.y = 0.0;
            ROBOT.angular.z = std::max(-MAX_ANGULAR_SPEED, std::min(MAX_ANGULAR_SPEED, vitesse)); // saturation
            pub_avancer.publish(ROBOT);
            rate.sleep(); // une pause 
        }

    }

    arreter();
    Duration(1).sleep();

}

/* Fonction GoToPoint(), récupére les cordonnés cartésiens x et y et
 * dessine une trajectoire pour y aller directement.
 */
void GoToPoint(double x2, double y2) {
    double x1 = pos_x; // position linéaire x initiale
    double y1 = pos_y; // position linéaire y initiale
    spinOnce(); // mise à jour des données odom

    // calculer l'angle 
    double angle = atan2((y2 - y1), (x2 - x1));

    rotation_Continue(angle * 180 / PI); // angle après calcul

    // calculer la distance 
    double distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    avancer_Continue(distance); // distance après calcul

}

int main(int argc, char** argv) {

    signal(SIGINT, sigintHandler);
    ros::init(argc, argv, "controleur", ros::init_options::NoSigintHandler);
    ros::init(argc, argv, "keyboard", ros::init_options::NoSigintHandler);

    NodeHandle nh;
    nh.getParam("freq", freq);

    //Publishers
    pub_msg = nh.advertise<String>("/log", 2); // Le publisher
    pub_avancer = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10); // Le publisher
    pub_reset = nh.advertise<std_msgs::Empty>("/reset", 2); // Le publisher

    //Subscribers
    sub_key = nh.subscribe("/keyboard_input", 1000, keyboardCallback); //Subscriber
    sub_pose = nh.subscribe("/odom", 10, PoseCallback); //Subscriber

    Rate loop_rate(frequence);

    while (ok()) {

        //ROS_INFO("Mode : %d", mode);

        //move(mode);
        //mode++;

        //if (mode > 2){
        //    mode = 0;
        //}

        if (num_key == 97) {
            Auto = !Auto;
            ROS_INFO("Auto = %d", Auto);
        }
        else if (num_key == 114) { //appuie r
            std_msgs::Empty empty;
            pub_reset.publish(empty);
            ROS_INFO("RESET");
        }

        if (Auto) {
            ROS_INFO("J'AI UNE CONSCIENCE");
            switch (num_key) {

            case(49): // appuie 1
                avancer_Continue(1.0);
                arreter();
                Duration(5).sleep();
                break;
            case(116): // appuie t
                rotation_Continue(45); // 45 deg (sens trigonométrique, vers la gauche)
                Duration(5).sleep();
                break;
            case(112): // appuie p
                GoToPoint(1.0, -1.0); // (x,y) coordonner cartésien
                arreter();
                break;
            }
        }

        else {
            ROS_INFO("APPUYEZ SUR ZQSD");
            switch (num_key) {

            case(122): //appuie z
                avancer(LINEAR_SPEED);
                ROS_INFO("J'AVANCE");
                break;
            case(115): //appuie s
                reculer(LINEAR_SPEED);
                ROS_INFO("JE RECULE");
                break;
            case(113): //appuie q
                tourner_g(ANGULAR_SPEED);
                ROS_INFO("JE TOURNE A GAUCHE");
                break;
            case(100): //appuie d
                tourner_d(ANGULAR_SPEED);
                ROS_INFO("JE TOURNE A DROITE");
                break;
            }

        }

        num_key = 32; //Valeur arbitraire pour ne pas garder la même valeur pour num_key (mettre une valeur inutilisé)
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
