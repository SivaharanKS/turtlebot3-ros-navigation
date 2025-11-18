#include <algorithm>
#include <ros/ros.h>
#include <string>
#include <vector>
#include <sensor_msgs/LaserScan.h>
#include <visualization_msgs/MarkerArray.h>
#include <visualization_msgs/Marker.h>
#include <limits>

using namespace ros;
using namespace std;

// VARIABLES GLOBALES
int NBR_REGION = 8;
int SCAN_SAMPLE = 360;
int ANGLE_REGION = 45;
std::vector<float> min_distances;

// PROTOTYPE DES FONCTIONS
float get_minimum(vector<float> data, int debut, int fin, int *indiceMin);

// FONCTION CALLBACK
// A COMPLETER//
void scanCallback(const sensor_msgs::LaserScan::ConstPtr& scan_in) {
  min_distances.clear();

  // A COMPLETER
  // Minimum dans chaque région => détection de l'obstacle / région

  int total_samples = scan_in->ranges.size(); // nombre total de mesures
  int samples_per_region = total_samples / NBR_REGION; // mesures par région
  float seuil_obstacle = 0.5; // seuil de détection d'obstacle en mètres

  for (int i = 0; i < NBR_REGION; i++) {
    int debut = i * samples_per_region;
    int fin = (i + 1) * samples_per_region - 1;
    int indice_min = -1;

    // Calcul du minimum dans la région i
    float min_region_i = get_minimum(scan_in->ranges, debut, fin, &indice_min);

    // Vérification de la validité de la mesure
    if (min_region_i < scan_in->range_min || min_region_i > scan_in->range_max) {
      min_region_i = FLT_MAX; // mesure invalide
    }

    // Détection d'obstacle
    if (min_region_i < seuil_obstacle) {
      ROS_INFO("Obstacle détecté dans la région %d à %.2f m", i, min_region_i);
    }
    // Stockage pour visualisation
    min_distances.push_back(min_region_i);
  }
}

int main(int argc, char **argv) {
  init(argc, argv, "detector");
  NodeHandle nh;

  Rate loop_rate(10 /* A COMPLETER */); // 10Hz

  // A COMPLETER
  // Subscriber odom_sub = nh.subscribe("/??", 100, scanCallback);
  Subscriber scan_sub = nh.subscribe("/scan", 100, scanCallback);

  spin();

  return 0;
}

float get_minimum(vector<float> data, int debut, int fin, int *indice_min) {

  float min = FLT_MAX;

  for (int i = debut; i <= fin; i++) {
    if (data[i]<min && data[i]> 0.0) {
      min = data[i];
      *indice_min = i;
    }
  }
  return std::min(10.0f, min);
}