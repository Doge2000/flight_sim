#include <iostream>
#include <fstream>
int main(){
    // rocket parameters
    double mass = 50.0;
    double fuel = 20.0;
    double thrust = 200.0;
    double burnrate = 1;
    double area = 0.1;
    double drag = 0.5;
    
    double dt = 0.001;
    double g = 0.8;
    double alt = 0;
    double v = 0;
    double time = 0;

    std::ofstream out("sim.csv");
    out<<"Time,Altitude,Velocity,Fuel\n";

    while(alt>=0){
        double dragforce = 0.5*1.225*drag*area*v*v; //drag force formula 1.225 is the density of the air

        if(v<0) dragforce = -dragforce; //adjust drag direction

        double mass = mass+fuel; //total mass

        out<<time<<","<<alt<<","<<v<<","<<fuel<<"\n";
        double accel = (thrust - dragforce - mass*g)/mass; //net acceleration

        v = v + accel*dt; //update velocity
        alt = alt + v*dt; //update altitude

        if(fuel>0){
            fuel = fuel - burnrate*dt; //update fuel
            if(fuel<0) fuel = 0;
        } else {
            thrust = 0; //no fuel, no thrust
        }
        if(int(time*1000)%100==0){
            std::cout << "Time: " << time << " s, Altitude: " << alt << " m, Velocity: " << v << " m/s, Fuel: " << fuel << " kg" << std::endl;
        }
        time = time + dt;
    }
    out.close();
    std::cout << "Rocket has landed after " << time << " seconds.\n";


}