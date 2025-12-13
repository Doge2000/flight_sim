#include <iostream>
#include <fstream>
#include <cmath>

struct Stage{
    double dry_mass;
    double fuel;
    double thrust;
    double burn_rate;
};

Stage stage1 = {0.05, 0.02, 10, 0.01};
Stage stage2 = {0.03, 0.01, 100, 0.005};

double airDensity(double altitude){
    //simplified model: decrease density with altitude
    return 1.225 * std::exp(-altitude/8500.0); //scale height of atmosphere ~8500m
};

int main(){

    double dt = 0.01;
    double g = 9.81;
    double alt = 0;
    double v = 0;
    double time = 0;

    //Model Rocket Parameters 
    double area = 0.00113; //m^2
    double dragcoeff = 0.75;
    

    Stage stages[2] = {stage1, stage2};
    int currentStage = 0;

    std::ofstream out("sim.csv");
    out<<"Time,Altitude,Velocity,Fuel,Stage\n";

    while(true){
        if(alt<=0 && time >2.0) break; 


        double rho = airDensity(alt); //air density at current altitude
        double dragforce = 0.5*rho*dragcoeff*area*v*v; //drag force formula 1.225 is the density of the air

        Stage& stage = stages[currentStage];
        if(v<0) dragforce = -dragforce; //adjust drag direction

        double currentmass;
        if (currentStage == 0) {
            currentmass = stages[0].dry_mass + stages[0].fuel
                       + stages[1].dry_mass + stages[1].fuel;
        } else {
            currentmass= stages[1].dry_mass + stages[1].fuel;
        }

        double thrust = (stage.fuel>0) ? stage.thrust : 0; //thrust only if fuel remains

        
        double accel = (thrust - dragforce - currentmass*g)/currentmass; //net acceleration

        v = v + accel*dt; //update velocity
        alt = alt + v*dt; //update altitude

        if(stage.fuel>0){
            stage.fuel = stage.fuel - stage.burn_rate*dt; //update fuel
            if(stage.fuel<0) {
                stage.fuel = 0;
            }
        } 
        else if(currentStage<1){
            currentStage++;
            std::cout << "Stage 2 ignited at time " << time << " seconds.\n";
        } 
        
        out<<time<<","<<alt<<","<<v<<","<<stage.fuel<< "," << currentStage+1 << "\n";
        time = time + dt;
    }
    out.close();
    std::cout << "Rocket has landed after " << time << " seconds.\n";


}

