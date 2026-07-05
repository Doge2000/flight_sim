#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <stack>
#include <string> 
#include <vector>

struct Stage {
    double dry_mass;
    double fuel;
    double thrust;
    double burn_rate;
};



int main(int argc, char* argv[]) {

    int numstages = std::stoi(argv[1]);
    std::vector<Stage> stages(numstages);


    int ind = 2;
    for(int i=0; i<numstages; i++){
        stages[i].thrust = std::stod(argv[ind++]);
        stages[i].fuel = std::stod(argv[ind++]);
        stages[i].dry_mass = std::stod(argv[ind++]);
        int time = std::stod(argv[ind++]);
        stages[i].burn_rate = stages[i].fuel / time;
    }

   
    
    double x = 1e-6, y = 1e-6;
    double vx = 1e-6, vy = 1e-6;
    double angle = M_PI / 2; // straight up

    double dt = 0.01;
    double g = 9.81;
    double alt = 0.0;
    double v = 0.0;
    double time = 0.0;

    bool chute = false;

    double area = 0.00113; // m^2
    double dragcoeff = 0.5;

    double prevy = 0;
    bool sentlo = false;
    bool sentap = false;
    bool sentfall= false;

   

    int currentStage = 0;


    while (true) {
        if (y <= 0.0 && time > 2.0) {
            y = 0.0;
            vy = 0.0;
            std:: cout << "STATUS: Landed\n";
            break;
        }

        if(time>3){
            angle-= 0.01*M_PI/180; //pitch program
        }



        if(!sentlo && time > 1 && vy >0){
            std:: cout << "STATUS: Powered Ascent\n";
            std:: cout.flush();
            sentlo = true;
        }

        if(!sentfall && sentlo && currentStage <= stages.size() - 1 && stages[currentStage].fuel <= 0.0){
            std::cout << "STATUS: Free Fall\n";
            std::cout.flush();
            sentfall = true;
        }

        if(!sentap && prevy >= 0 && vy <= 0  && time>2.0){
            std:: cout << "STATUS: Apogee Reached\n";
            std:: cout.flush();
            sentap = true;
        }

        if(vy<0 && y<=200.0 && !chute){
            chute = true;
            std::cout << "STATUS: Parachute deployed\n";
            std::cout.flush();
        }

        
        


        Stage& stage = stages[currentStage];

       

        double currentmass = 0;
        for(int i=0; i<stages.size(); i++){
            currentmass += stages[i].dry_mass + stages[i].fuel;
        }
        double airdensity = 1.225;
        double Fdragnochute = 0.5 * airdensity * vy * vy * area * dragcoeff;
        double Fdragwchute = 0.5 * airdensity * vy * vy * 0.46 * 1.5; //about what a parachute drag coef would be 
        double currentdrag = chute ? Fdragwchute : Fdragnochute;

        double Fx = 0;
        double Fy = 0;
        //if theres fuel then apply thrust
        if(stage.fuel > 0.0){
            Fx = stage.thrust * std::cos(angle);
            Fy = stage.thrust * std::sin(angle) - currentmass * g;
        }
        //other wise just gravity
        else{
            Fx = 0;
            Fy = - currentmass * g;
        }

        //drag is always opposite to velocity
        if(vy<0){
            Fy+= currentdrag;
        }
        else{
            Fy -= currentdrag;
        }

        double ax = Fx / currentmass;
        double ay = Fy / currentmass;

        vx += ax * dt;
        vy += ay * dt;
        x += vx * dt;
        y += vy * dt;
        alt = y;
        v = std::sqrt(vx*vx + vy*vy);
       
        if (stage.fuel > 0.0) {
            stage.fuel -= stage.burn_rate * dt;
            if (stage.fuel < 0.0){
                stage.fuel = 0.0;
            }
        } else if (currentStage < stages.size() - 1) {
            stages[currentStage].dry_mass = 0.0; 
            currentStage++;
        }

      
        std::cout << time << "," << x << "," << y << ","
            << vx << "," << vy << "," << v << ","
            << stage.fuel << "," << currentmass << "," << angle << "," << (currentStage+1) << "\n";
        std::cout.flush();
        time += dt;
        prevy = vy;
        

    }
   
    std::cout << "Rocket has landed after " << time << " seconds.\n";
}
