#ifndef LIB_ENCODER_H
#define LIB_ENCODER_H

#include "Arduino.h"

class CStateMachineRotary // déclaration de la classe
{
public: // membres accessibles depuis l'extérieur de la classe, il s'agit de
        // l'interface d'interaction de la classe
  CStateMachineRotary(
      int brocheAinit,
      int brocheBinit); // Constructeur avec paramètres pour régler les 2
                        // broches Arduino utilisées
  void
  clock(); // méthode pour cadencer la machine à état (faire 1 coup d'horloge)
  int getPosition() // méthode accesseur pour accéder à l'attribut privé
                    // position
  {
    return position;
  } // on parle de méthode inlinée car l'implémentation est faite dans la
    // déclaration de la classe

private:     // membres privés pour réaliser l'encapsulation: ces attributs sont
             // inacessibles directement depuis l'extérieur de la classe
  byte etat; // numéro de l'état actif
  byte entreeA; // valeur lue sur l'entrée A
  byte entreeB; // valeur lue sur l'entrée B
  int brocheA;  // numéro de broche Arduino utilisée pour l'entrée A
  int brocheB;  // numéro de broche Arduino utilisée pour l'entrée B
  virtual void frontDetecte(int direction) {
  }; // methode abstraite, sera implémentée dans la classe fille
  virtual void actionSurTousLesEtats() {
  }; // methode abstraite, sera implémentée dans la classe fille
protected:      // ces membres peuvent être accédés dans les classes filles
  int position; // position angulaire mesurée
}; // ne pas oublier le ;

class CStateMachineRotaryWithSpeed : public CStateMachineRotary {
public:
  CStateMachineRotaryWithSpeed(int brocheAinit, int brocheBinit);
  float getSpeed(); // méthode accesseur pour accéder à l'attribut privé speed

private:
  float speed;                 // vitesse mesurée
  unsigned long lastPulseTime; // horodatage du dernier front
  void frontDetecte();         // implémentation pour effectuer des
                               // traitements lorsqu'un front est detecté
};

#endif
