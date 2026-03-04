#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

struct Stage {
    double dry_mass;
    double fuel;
    double thrust;
    double burn_rate;
};



double airDensity(double altitude) {
    return 1.225 * std::exp(-altitude / 8500.0);
}

int main(int argc, char* argv[]) {
    double stage1thrust = 10.0; // N
    double stage2thrust = 10.0; // N
    double stage1fuel = 50.0; // kg
    double stage2fuel = 30.0; // kg
    double stage1mass = 20.0; // kg
    double stage2mass = 15.0; // kg
    if (argc > 1) {
        stage1thrust = std::stod(argv[1]);
        stage2thrust = std::stod(argv[2]);
        stage1fuel = std::stod(argv[3]);
        stage2fuel = std::stod(argv[4]);
        stage1mass = std::stod(argv[5]);
        stage2mass = std::stod(argv[6]);
    }

    Stage stage1 = {stage1mass, stage1fuel, stage1thrust, 0.01};
    Stage stage2 = {stage2mass, stage2fuel, stage2thrust, 0.005};
    double x = 0.0, y = 0.0;
    double vx = 0.0, vy = 0.0;
    double angle = 90.0 * M_PI / 180.0;

    double dt = 0.01;
    double g = 9.81;
    double alt = 0.0;
    double v = 0.0;
    double time = 0.0;

    bool chute = false;

    double area = 0.00113; // m^2
    double dragcoeff = 0.1;

    Stage stages[2] = {stage1, stage2};
    int currentStage = 0;

    std::ofstream out("sim.csv");
    out << std::fixed << std::setprecision(6);
    out << "Time,X,Y,Vx,Vy,Speed,Fuel,Mass,Angle,Stage\n";

    while (true) {
        if (time > 3600.0) {
            std::cout << "Simulation timed out\n";
            break;
        }

        if (y <= 0.0 && time > 2.0) {
            y = 0.0;
            vy = 0.0;
            break;
        }


        Stage& stage = stages[currentStage];

        if(vy<0 && y<200.0 && !chute){
            chute = true;
            std::cout << "Parachute deployed at time " << time << " seconds.\n";
        }

        double Cd_eff = chute?  0.5 : dragcoeff;
        double area_eff = chute? 0.1 : area;
        if(stage.fuel>0 && angle > M_PI/4){
            angle -= 0.1*dt;
            if(angle < 0) angle = 0;
        }



        
      
        double currentmass;
        if (currentStage == 0) {
            currentmass = stages[0].dry_mass + stages[0].fuel
                        + stages[1].dry_mass + stages[1].fuel;
        } else {
            currentmass = stages[1].dry_mass + stages[1].fuel;
        }

        double thrust = (stage.fuel > 0.0) ? stage.thrust : 0.0;

         v = std::sqrt(vx*vx + vy*vy);
     
        double rho = airDensity(y);
        double Fx_drag = 0.0, Fy_drag = 0.0;
        double drag_mag = 0.5 * rho * Cd_eff * area_eff * v * v;
        if (v > 1e-9) {
            Fx_drag = -drag_mag * (vx / v);
            Fy_drag = -drag_mag * (vy / v);
        }

  
        double Fx_thrust = thrust * std::cos(angle);
        double Fy_thrust = thrust * std::sin(angle);


        double Fx = Fx_thrust + Fx_drag;
        double Fy = Fy_thrust + Fy_drag - currentmass * g;

        double ax = Fx / currentmass;
        double ay = Fy / currentmass;

        vx += ax * dt;
        vy += ay * dt;
        x += vx * dt;
        y += vy * dt;
        alt = y;

       
        if (stage.fuel > 0.0) {
            stage.fuel -= stage.burn_rate * dt;
            if (stage.fuel < 0.0){
                stage.fuel = 0.0;
                std::cout << "Stage " << (currentStage+1) << " has run out of fuel at time " << time << " seconds.\n";
            }
        } else if (currentStage < 1) {
            stages[0].dry_mass = 0.0; 
            currentStage++;
            std::cout << "Stage 2 ignited at time " << time << " seconds.\n";
        }

      
        out << time << "," << x << "," << y << ","
            << vx << "," << vy << "," << v << ","
            << stage.fuel << "," << currentmass << "," << angle << "," << (currentStage+1) << "\n";

        time += dt;
    }

    out.close();
    std::cout << "Rocket has landed after " << time << " seconds.\n";
}
