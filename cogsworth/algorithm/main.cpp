#include <QDebug>

#include "act_algorithm.hpp"
#include "act_project.hpp"

int main() {
  qDebug() << "yaya";
  ActProject project;
  project.SetProjectName("yaya");
  ActAlgorithm computer(project.GetProjectName());
  return 0;
}
