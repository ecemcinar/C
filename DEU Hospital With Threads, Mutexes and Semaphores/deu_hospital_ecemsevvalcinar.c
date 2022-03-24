#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <time.h>


int REGISTRATION_SIZE = 10;
int RESTROOM_SIZE = 10;
int CAFE_NUMBER = 10;
int GP_NUMBER = 10;
int PHARMACY_NUMBER = 10;
int BLOOD_LAB_NUMBER = 10;
int OR_NUMBER = 10;
int SURGEON_NUMBER = 30;
int NURSE_NUMBER = 30;
int SURGEON_LIMIT = 5;
int NURSE_LIMIT = 5;

int HOSPITAL_WALLET = 0;
int WAIT_TIME = 100;
int REGISTRATION_TIME = 100;
int GP_TIME = 200;
int PHARMACY_TIME = 100;
int BLOOD_LAB_TIME = 200;
int SURGERY_TIME = 500;
int CAFE_TIME = 100;
int RESTROOM_TIME = 100;

int REGISTRATION_COST = 100;
int PHARMACY_COST = 200; // Calculated randomly between 1 and given value.
int BLOOD_LAB_COST = 200;
int SURGERY_OR_COST = 200;
int SURGERY_SURGEON_COST = 100;
int SURGERY_NURSE_COST = 50;
int CAFE_COST = 200; // Calculated randomly between 1 and given value.

// The global increase rate of hunger and restroom needs of patients. It will increase
//randomly between 1 and given rate below.
int HUNGER_INCREASE_RATE = 10;
int RESTROOM_INCREASE_RATE = 10;

int Hunger_Meter=100; // Initialized between 1 and 100 at creation.
int Restroom_Meter=100; // Initialized between 1 and 100 at creation.

int PATIENT_NUMBER = 1000;

struct Patient{
    int patientID;
    char ilness[30];
    int perOfHunger;
    int perOfRestroomNeeds;
    int isSeenByGP;
    int isWentToBLab;
    int isWentToSurgery;
    int nurseNeed;
    int surgeonNeed;
};
typedef struct Patient Patient;

sem_t Registration;
sem_t Restroom;
sem_t Cafe;
sem_t GeneralPractitioner;
sem_t Pharmacy;
sem_t BloodLab;
sem_t SurgergyRoom;
pthread_mutex_t mutex;

char illnesses[15][30] = {"Influenza","Diphtheria","Anxiety Disorders","Miscarriage","Bulimia Nervosa",
"Insomnia Disorder","Major Depressive Disorder","Heart Failure","Arrhythmia","Allergic Rhinitis","Itching",
"Kidney Cancer","Liver Cancer","Meningitis","Pancreatic Cancer"};


void determineValues(Patient *patient){ 
    // hunger and restroom needs have been determined.
    patient->perOfHunger=rand()%Hunger_Meter+1;
    patient->perOfRestroomNeeds=rand()%Restroom_Meter+1;
    strcpy(patient->ilness,illnesses[rand()%15]); // giving a disorder to patient randomly
    patient->isSeenByGP=0; // first, patients come to the hospital without be seen by any department, these values keepen as true (1),false (0)
    patient->isWentToBLab=0; 
    patient->isWentToSurgery=0;
    patient->surgeonNeed=0;
    patient->nurseNeed=0;
}
void determineValuesOfNurseAndSurgeonNeed(Patient *patient){ // randomly selected nurse and surgeon number which will be in patient's surgery
    int nurseNeed = rand()%NURSE_LIMIT+1;
    int doctorNeed = rand()%SURGEON_LIMIT+1;
    patient->surgeonNeed=doctorNeed;
    patient->nurseNeed=nurseNeed;
}
void wentToSurgery(Patient *patient){
    patient->isWentToSurgery=1;
}
void wentToBloodLab(Patient *patient){
    patient->isWentToBLab=1;
}
void seenByDoctor(Patient *patient){
    patient->isSeenByGP=1;
}
void increasePerOfHunger(Patient *patient){
    patient->perOfHunger+= rand()%HUNGER_INCREASE_RATE+1;
}
void increasePerOfRestroom(Patient *patient){
    patient->perOfRestroomNeeds+= rand()%RESTROOM_INCREASE_RATE+1;
}
void ResetPerOfHunger(Patient *patient){
    patient->perOfHunger=0;
}
void ResetPerOfRestroom(Patient *patient){
    patient->perOfRestroomNeeds=0;
}
void goToRestroom(Patient *patient){
    if(sem_trywait(&Restroom)==0){
        printf("Patient (%d) has entered Restroom --> RestroomRate:%d\n",patient->patientID,patient->perOfRestroomNeeds);
        int timeSpent = rand()%RESTROOM_TIME+1;
        usleep(timeSpent*1000);
        //reset restroom percent
        //restroom is no cost
        ResetPerOfRestroom(patient);
        printf("Patient (%d) has left the Restroom --- TimeSpent:%d --- RestroomRate:%d\n",patient->patientID,timeSpent,patient->perOfRestroomNeeds);
        sem_post(&Restroom);
    }
    else{ // if restroom is full, patient has to wait
        printf("Patient (%d) is waiting for Restroom to enter --> RestroomRate:%d\n",patient->patientID,patient->perOfRestroomNeeds);
        int waitTime= rand()%WAIT_TIME+1;
        usleep(waitTime*1000);
        sem_wait(&Restroom);

        printf("Patient (%d) has entered Restroom after waiting %d  --> RestroomRate:%d\n",patient->patientID,waitTime,patient->perOfRestroomNeeds);
        
        int timeSpent=rand()&RESTROOM_TIME+1;
        usleep(timeSpent*1000);
        ResetPerOfRestroom(patient);
        printf("Patient (%d) has left the Restroom --- TimeSpent:%d --- RestroomRate:%d\n",patient->patientID,timeSpent,patient->perOfRestroomNeeds);
        sem_post(&Restroom); // isi biten ciksin
    }
    
}
void goToCafe(Patient *patient){
    
    if(sem_trywait(&Cafe)==0){
        printf("Patient (%d) has entered Cafe --> HungerRate:%d \n",patient->patientID,patient->perOfHunger);
        int time = rand()%CAFE_TIME+1;
        usleep(time*1000);
            
        // add the price to Total Hospital Wallet which patient spent in Cafe
        int moneySpent=rand()%CAFE_COST+1;
        HOSPITAL_WALLET+=moneySpent;
        ResetPerOfHunger(patient); //reset the hunger percent
        printf("Patient (%d) has left the Cafe --- TimeSpent:%d --- MoneySpent:%d --- HungerRate:%d \n",patient->patientID,time,moneySpent,patient->perOfHunger);
        sem_post(&Cafe);
    }
    else{ // cafe is full, patient has to wait

        // while waiting for cafe, patients can go to restroom
        increasePerOfRestroom(patient);
        if(patient->perOfRestroomNeeds>99){
            goToRestroom(patient);
        }
        printf("Patient (%d) is waiting for Cafe to enter\n",patient->patientID);
        int waitTime= rand()&WAIT_TIME+1;
        usleep(waitTime*1000);
            
        sem_wait(&Cafe);
        printf("Patient (%d) has entered Cafe after waiting %d --> HungerRate:%d \n",patient->patientID,waitTime,patient->perOfHunger);
        int timeSpent=rand()%CAFE_TIME+1;
        usleep(timeSpent*1000);
        int moneySpent=rand()%CAFE_COST+1;
        HOSPITAL_WALLET+=moneySpent;
        ResetPerOfHunger(patient);   //reset the hunger percent
        printf("Patient (%d) has left the Cafe --- TimeSpent:%d --- MoneySpent:%d --- HungerRate:%d \n",patient->patientID,time,moneySpent,patient->perOfHunger);
        sem_post(&Cafe); // isi biten ciksin

    }
 
   
}

void goToSurgery(Patient *patient){
    int myBoolean=0;
    if(sem_trywait(&SurgergyRoom)==0){ // it means there are empty space in surgery room (semaphore) and patient can go there
        
        if(patient->nurseNeed==0 && patient->surgeonNeed==0){ // onceden belirlendi ise, tekrardan belirlenmesini engelliyorum
            // its because the function I wrote for the surgery part is recursive function, i have to determine these values just one time. not again and again
            determineValuesOfNurseAndSurgeonNeed(patient);
        }
        printf("Patient (%d)'s surgery will be start soon.For this surgery, %d nurses and %d surgeons are needed\n",patient->patientID,patient->nurseNeed,patient->surgeonNeed);
        while(myBoolean==0){ // while true
        
            if(patient->nurseNeed <= NURSE_NUMBER && patient->surgeonNeed<= SURGEON_NUMBER){
                NURSE_NUMBER-=patient->nurseNeed;
                SURGEON_NUMBER-=patient->surgeonNeed;
                pthread_mutex_lock(&mutex);
                printf("Patient (%d) has entered to surgery with %d nurses and %d surgeons\n",patient->patientID,patient->nurseNeed,patient->surgeonNeed);
                int surgeryCost= patient->surgeonNeed*SURGERY_SURGEON_COST + patient->nurseNeed*SURGERY_NURSE_COST;
                HOSPITAL_WALLET+=surgeryCost;
                int surgeryTime=rand()%SURGERY_TIME+1;
                usleep(surgeryTime*800);
                printf("Patient (%d)'s surgery has completed --- Time:%d --- Cost:%d\n",patient->patientID,surgeryTime,surgeryCost);
                pthread_mutex_unlock(&mutex);
                wentToSurgery(patient); // patient.isWentToSurgery will be 1, it prevents patient to go surgery room again after going to GP for second time
                //free them
                sem_post(&SurgergyRoom);
                // surgery is over, nurses and surgeons are available now
                NURSE_NUMBER+=patient->nurseNeed;
                SURGEON_NUMBER+=patient->surgeonNeed;
                myBoolean=1; // break the while loop
                break;
            }
            else{
                // if there is not enough nurse or surgeon, patient will wait for nurses and surgeons // thread in semaphore but waiting for nurses and surgeons
                printf("Available nurse number:%d --- Available surgeon number:%d\n",NURSE_NUMBER,SURGEON_NUMBER);
                printf("Patient (%d) is waiting for surgery in surgery room\n",patient->patientID);
                increasePerOfHunger(patient);
                if(patient->perOfHunger>99){
                    goToCafe(patient);
                }
                increasePerOfRestroom(patient);
                if(patient->perOfRestroomNeeds>99){
                    goToRestroom(patient);
                }
                int waitTime = rand()%WAIT_TIME+1;
                usleep(waitTime*2000);
            }
        } 
    }
    else{ // it means there are no space in surgery room (semaphore) and patient-thread cannot go there
        printf("Patient (%d) is waiting for surgery room to enter --- HungryRate:%d ---- RestroomRate:%d\n",patient->patientID,patient->perOfHunger,patient->perOfRestroomNeeds);
        // while waiting increase the hunger and restroom needs
        int waitTime=rand()%WAIT_TIME+1;
        sleep(waitTime/75);
        increasePerOfHunger(patient);
        if(patient->perOfHunger>99){
            goToCafe(patient);
        }
        increasePerOfRestroom(patient);
        if(patient->perOfRestroomNeeds>99){
            goToRestroom(patient);
        }
        waitTime=rand()%WAIT_TIME+1;
        usleep(waitTime*2000);
        //recursively 
        // it will look the semaphore if there are any place till it find a place in semaphore
        goToSurgery(patient);
    } 
}

void goToPharmacy(Patient *patient){
    if(sem_trywait(&Pharmacy)==0){
        printf("Patient (%d) has entered to Pharmacy Store --- Illness:%s\n",patient->patientID,patient->ilness);
        int pharmTime= rand()%PHARMACY_TIME+1;
        usleep(pharmTime*1000);
        int pharmacyCost= 1+rand()%PHARMACY_COST;  // it wont be added to total hospital wallet
        printf("Patient (%d) has left Pharmacy Store --- Medicine Cost:%d --- TimeSpent:%d\n",patient->patientID,pharmacyCost,pharmTime);
        sem_post(&Pharmacy);
    }
    else{
        printf("Patient (%d) is waiting for Pharmacy Store to enter --- Illness:%s\n --- HungerRate:%d --- RestroomRate:%d\n",patient->patientID,patient->ilness,patient->perOfHunger,patient->perOfRestroomNeeds);
        // while waiting, increase hunger and restroom, if>99, go to cafe or restroom
        increasePerOfHunger(patient);
        if(patient->perOfHunger>99){
            goToCafe(patient);
        }
        increasePerOfRestroom(patient);
        if(patient->perOfRestroomNeeds>99){
            goToRestroom(patient);
        }
        int waitTime= rand()&WAIT_TIME+1;
        
        usleep(waitTime*1000);
        sem_wait(&Pharmacy);
        printf("Patient (%d) has entered to Pharmacy after waiting %d --- Illness:%s --- HungerRate:%d --- RestroomRate:%d\n",patient->patientID,waitTime,patient->ilness,patient->perOfHunger,patient->perOfRestroomNeeds);
        int pharmTime= rand()%PHARMACY_TIME+1;
        usleep(pharmTime*1000);
        int pharmacyCost= 1+rand()%PHARMACY_COST;  // it wont be added to total hospital wallet
        printf("Patient (%d) has left Pharmacy Store --- Medinice Cost:%d --- TimeSpent:%d\n",patient->patientID,pharmacyCost,pharmTime);
        sem_post(&Pharmacy);
    }
    // there is no place to go after leaving the pharmacy
}
void goToBloodLab(Patient *patient){
   
    if(sem_trywait(&BloodLab)==0){
        printf("Patient (%d) has entered to Blood Lab --- Illness:%s\n",patient->patientID,patient->ilness);
        int BLTime=rand()%BLOOD_LAB_TIME+1;
        usleep(BLTime*1000);
        // Blood Lab Cost is constant, it does not select randomly
        HOSPITAL_WALLET+=BLOOD_LAB_COST; // add cost to the hospital wallet
        printf("Patient (%d) has left Blood Lab and will enter GP again --- TimeSpentInGP:%d --- Illness:%s --- BloodLabCost:%d\n",patient->patientID,BLTime,patient->ilness,BLOOD_LAB_COST);
        sem_post(&BloodLab);
    }
    else{
        printf("Patient (%d) is waiting for Blood Lab to enter --- Hungry Rate:%d --- Restroom Rate:%d ---- Illness:%s\n",patient->patientID,patient->perOfHunger,patient->perOfRestroomNeeds,patient->ilness);
        // while waiting, increase
        increasePerOfHunger(patient);
        if(patient->perOfHunger>99){
            goToCafe(patient);
        }
        increasePerOfRestroom(patient);
        if(patient->perOfRestroomNeeds>99){
            goToRestroom(patient);
        }

        int waitTime= rand()&WAIT_TIME+1;
        usleep(waitTime*1000);
        sem_wait(&BloodLab);
        printf("Patient (%d) has entered to Blood Lab after waiting %d --- Illness:%s --- Hungry Rate:%d --- Restroom Rate:%d\n",patient->patientID,patient->ilness,waitTime,patient->perOfHunger,patient->perOfRestroomNeeds);
        int BLTime=rand()%BLOOD_LAB_TIME+1;
        usleep(BLTime*1000);
        HOSPITAL_WALLET+=BLOOD_LAB_COST;
        printf("Patient (%d) has left Blood Lab and will enter GP again --- TimeSpentInGP:%d --- Illness:%s --- BloodLabCost:%d\n",patient->patientID,BLTime,patient->ilness,BLOOD_LAB_COST);
        sem_post(&BloodLab);
    }
    
    wentToBloodLab(patient); // it prevents to patient go to blood lab for the second time.
    //printf("Patient (%d) In Blood Lab is went:%d\n",patient->patientID,patient->isWentToBLab);
}

void goToGeneralPractitioner(Patient *patient){

    if(sem_trywait(&GeneralPractitioner)==0){
        printf("Patient (%d) has entered to GP --- Illness:%s\n",patient->patientID,patient->ilness);
        int GPTime = rand()%GP_TIME+1;
        usleep(GPTime*1000);
        printf("Patient (%d) has left GP --- TimeSpentInGP:%d --- Illness:%s\n",patient->patientID,GPTime,patient->ilness);
        sem_post(&GeneralPractitioner);
    }
    else{
        // while waiting for GP, patients can go either restroom or cafe
        printf("Patient (%d) is waiting for GP --- Hungry Rate:%d --- Restroom Rate:%d --- Illness:%s\n",patient->patientID,patient->perOfHunger,patient->perOfRestroomNeeds,patient->ilness);
        increasePerOfHunger(patient);
        if(patient->perOfHunger>99){
            goToCafe(patient);
        }
        increasePerOfRestroom(patient);
        if(patient->perOfRestroomNeeds>99){
            goToRestroom(patient);
        }

        int waitTime= rand()&WAIT_TIME+1;
        usleep(waitTime*1000);

        sem_wait(&GeneralPractitioner); // thread comes in semaphore
        printf("Patient (%d) has entered to GP after waiting %d --- Illness:%s --- Hungry Rate:%d --- Restroom Rate:%d\n",patient->patientID,waitTime,patient->ilness,patient->perOfHunger,patient->perOfRestroomNeeds);
        int GPTime = rand()%GP_TIME+1;
        usleep(GPTime*1000);
        printf("Patient (%d) has left GP --- TimeSpentInGP:%d --- Illness:%s\n",patient->patientID,GPTime,patient->ilness);
        sem_post(&GeneralPractitioner);
    }
    // after gp, it has to decide where to go
    int decisionAfterGP=rand()%3;
    if(decisionAfterGP==0){ // go to pharmacy
        seenByDoctor(patient); // there is no need to go GP again
        goToPharmacy(patient);
    }
    else if(decisionAfterGP==1 && patient->isWentToBLab==0 && patient->isWentToSurgery==0){ // blood lab
        goToBloodLab(patient); // in this function, patient's isWentToBLab value becomes 1 so that after returning gp, s/he will not go to lab again
        //printf("Patient (%d) iswentbloodlab:%d\n",patient->patientID,patient->isWentToBLab);

        // After given blood, the patient returns to GP and they are either prescribed medicine if
        //they need it and goes to pharmacy as has been shown above or not if they do not require it.
        goToGeneralPractitioner(patient); // back to GP
    }
    else if(decisionAfterGP==2 && patient->isWentToBLab==0 && patient->isWentToSurgery==0){ // go to surgery
        // patient.isWentToBLab==0 means, there is only 2 options after the lab. patient can go the pharmacy store but cannot go surgery 
        // so, if iswenttolab==1, it means patient has already gone lab and cannot go surgery room
        goToSurgery(patient);
        // after surgery, go to gp again
        goToGeneralPractitioner(patient);
    }
    else{
        //go home, it means patients left the hospital
    }

}
void* goToRegistration (void* args){
    Patient patient;
    patient.patientID=*(int*)args;

    // determines the first values of patient such as illnes and percent of both restroom and hunger randomly
    determineValues(&patient);
    //printf("Patient ID:%d | | Hungry Rate:%d | | Restroom Rate:%d | | Illness:%s\n",patient.patientID,patient.perOfHunger,patient.perOfRestroomNeeds,patient.ilness);
    

    if(sem_trywait(&Registration)==0){ // semaphore is not full
        printf("Patient (%d) has entered to Registration --- Hungry Rate:%d --- Restroom Rate:%d --- Illness:%s\n",patient.patientID,patient.perOfHunger,patient.perOfRestroomNeeds,patient.ilness);
        HOSPITAL_WALLET+=REGISTRATION_COST;
        int regTime=rand()%REGISTRATION_TIME+1;
        usleep(regTime*1000);
        printf("Patient (%d) has registered --- RegTime:%d --- RegCost:%d \n",patient.patientID,regTime,REGISTRATION_COST);
        sem_post(&Registration); // isi biten ciksin
    }
    else{ // semaphore is full
        printf("Patient (%d) is waiting for Registration --- Hungry Rate:%d --- Restroom Rate:%d --- Illness:%s\n",patient.patientID,patient.perOfHunger,patient.perOfRestroomNeeds,patient.ilness);
        
        increasePerOfHunger(&patient); // increases the hunger percent of patient
        
        if(patient.perOfHunger>99){
            goToCafe(&patient);
        }
        increasePerOfRestroom(&patient); // increases the restroom need per of patient
        if(patient.perOfRestroomNeeds>99){
            goToRestroom(&patient);
        }
        
        int waitTime= rand()&WAIT_TIME+1;
        
        usleep(waitTime*1000);
        sem_wait(&Registration);
        printf("Patient (%d) has entered to Registration after waiting %d\n",patient.patientID,waitTime);
        int regTime=rand()%REGISTRATION_TIME+1;
        usleep(regTime*1000);
        HOSPITAL_WALLET+=REGISTRATION_COST;
        printf("Patient (%d) has registered --- RegTime:%d --- RegCost:%d \n",patient.patientID,regTime,REGISTRATION_COST);
        sem_post(&Registration);

    }
    //After registration, patient has to go to GP
    goToGeneralPractitioner(&patient);
}

int main(int argc,char *argv[]){

    pthread_mutex_init(&mutex,NULL);

    // creating threads as patients
    pthread_t patients[PATIENT_NUMBER];
    
    sem_init(&Registration,0,REGISTRATION_SIZE);
    sem_init(&Cafe,0,CAFE_NUMBER);
    sem_init(&Restroom,0,RESTROOM_SIZE);
    sem_init(&GeneralPractitioner,0,GP_NUMBER);
    sem_init(&Pharmacy,0,PHARMACY_NUMBER);
    sem_init(&BloodLab,0,BLOOD_LAB_NUMBER);
    sem_init(&SurgergyRoom,0,OR_NUMBER);
    
    int i;
    for(i=0;i<PATIENT_NUMBER;i++){
        int* a=malloc(sizeof(int));
        *a=i;
        
        if(pthread_create(&patients[i],NULL,&goToRegistration,a)!=0){
            perror("Error occurs while creating the thread.");
            return 1;
        }
        if(i%50==0){ // it provides randomly time interval which patients come to the hospital
            usleep(rand()%(WAIT_TIME+1)*1000);
        }
        
    }
    for ( i = 0; i < PATIENT_NUMBER; i++){
        if(pthread_join(patients[i],NULL)!=0){
            return 2;
        }
    }
    printf("----------\nTotal Hospital Wallet: %d\n----------", HOSPITAL_WALLET);

    // destroying semaphores
    sem_destroy(&Registration);
    sem_destroy(&Cafe);
    sem_destroy(&Restroom);
    sem_destroy(&GeneralPractitioner);
    sem_destroy(&Pharmacy);
    sem_destroy(&BloodLab);
    sem_destroy(&SurgergyRoom);

    return 0;
}