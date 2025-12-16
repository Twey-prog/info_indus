#include "pilote_encodeur.h"

////////////////////////////////////////////////////////////////////
//Implémentation du constructeur de la classe fille, exécutée après avoir exécuté le constructeur de la classe mère
CStateMachineRotaryWithSpeed::CStateMachineRotaryWithSpeed(int brocheAinit, int brocheBinit):CStateMachineRotary(brocheAinit, brocheBinit){
    lastPulseTime=micros(); //initialisation de la date initiale pour l'horodatage des fronts
    speed=0; //initialise vitesse nulle au démarrage. Attention, en simu, l'encodeur génère des fronts parasites
}  
////////////////////////////////////////////////////////////////////
//implémentation de la méthode frontDetecte() qui n'était pas implémentée dans la classe mère
void CStateMachineRotaryWithSpeed::frontDetecte()
{   
    unsigned long currentTime = micros();
    this->speed = 1000000.0f/ (currentTime - this->lastPulseTime);
    this->lastPulseTime = currentTime;
    //Serial.println(currentTime);
    //Serial.println(this->lastPulseTime);   
}
//implémentation de la mesure de vitesse
float CStateMachineRotaryWithSpeed::getSpeed(){
    this->frontDetecte();
  //à faire: calculer la vitesse et l'affecter à l'attribut speed
    unsigned long newTime= micros();
    if((newTime-this->lastPulseTime)>4000000)
        speed = 0;
    return speed;
}


////////////////////////////////////////////////////////////////////
//Implémentation de la méthode constructeur
CStateMachineRotary::CStateMachineRotary(int brocheAinit, int brocheBinit){
// à compléter
    brocheA=brocheAinit;
    brocheB=brocheBinit;
    pinMode(brocheA, INPUT);
    pinMode(brocheB, INPUT);
    entreeA=digitalRead(brocheA);
    entreeB=digitalRead(brocheB);
    etat=2;
    position=0;

}
//Implémentation de la méthode clock
void CStateMachineRotary::clock(){
// à compléter
entreeA=digitalRead(brocheA);
entreeB=digitalRead(brocheB);


//actions sur transition
 if ((etat==0) && (entreeA==1)){
    position=position + 1;
 }

 if ((etat == 1) && (entreeA == 0)){
    position = position - 1 ;
 }
 

//évolution de l'état
 switch (etat){
	case 0: if (entreeA==1)
			etat=1;
		else if  (entreeB==1)
			etat=3;
		break;
    case 1: if(entreeA == 0)
            etat=0;
        else if (entreeB == 1)
            etat=2;
        break;
    case 2: if(entreeA == 0)
            etat = 3;
        else if (entreeB == 0)
            etat = 1;
        break;
    case 3: if(entreeA == 1)
            etat = 2;
        else if (entreeB == 0)
            etat = 0;
        break;
    default:
        break;
        }
}