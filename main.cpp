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

// struct PID{
//     double kp;
//     double ki;
//     double kd;
//     double integral = 0.0;
//     double prev_error = 0.0;

//     double update(double error, double dt) {
//         integral += error * dt;
//         double derivative = (error - prev_error) / dt;
//         prev_error = error;
//         return kp * error + ki * integral + kd * derivative;
//     }
// };



// double airDensity(double altitude) {
//     const double rho0 = 1.225;
//     const double H = 8500.0;
//     return rho0 * std::exp(-altitude / H);
// }

// double angleattack(double theta, double vx, double vy) {
//     double v = std::sqrt(vx*vx + vy*vy)+1e-9;
//     double alpha = std::atan2(vy, vx);
//     return theta - alpha;
// }

// double pitchProgram(double t){
//         if(t<2) return M_PI/2;
//         if(t<20){
//             double frac = (t-2)/18.0;
//             return M_PI/2+frac*(20*M_PI/180.0 - M_PI/2);
//         }
//         return 20*M_PI/180.0;
//     }

int main(int argc, char* argv[]) {

    std::ifstream infile("C:\\Users\\JadeL\\flight_sim\\config.txt");
   
    if (!infile.is_open()) {
        std::cerr <<"Error: Could not open config.txt. Please ensure the file exists and is in the correct location.\n";
        return 1;
    }
    double stage1thrust = 0.0;
    double stage2thrust = 0.0;
    double stage1fuel = 0.0;
    double stage2fuel = 0.0;
    double stage1mass = 0.0;
    double stage2mass = 0.0;

    infile >> stage1thrust >> stage2thrust >> stage1fuel >> stage2fuel >> stage1mass >> stage2mass;
    infile.close();


    // std::cout << stage1thrust << "," << stage2thrust << "," << stage1fuel << "," << stage2fuel << "," << stage1mass << "," << stage2mass << "\n";

    // PID pid = {3.0, 0.0, 1.5};
   if(argc >1 ){
    stage1thrust = std::stod(argv[1]);
    stage2thrust = std::stod(argv[2]);
    stage1fuel = std::stod(argv[3]);
    stage2fuel = std::stod(argv[4]);
    stage1mass = std::stod(argv[5]);
    stage2mass = std::stod(argv[6]);
   }
    

    Stage stage1 = {stage1mass, stage1fuel, stage1thrust, 0.01};
    Stage stage2 = {stage2mass, stage2fuel, stage2thrust, 0.005};
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

    std::vector<Stage> stages = {stage1, stage2};

   

    int currentStage = 0;

    std::ofstream out("sim.csv");
    out << std::fixed << std::setprecision(6);
    out << "Time,X,Y,Vx,Vy,Speed,Fuel,Mass,Angle,Stage\n";


    while (true) {
        if (time > 3600) {
            std::cout << "Simulation timed out\n";
            break;
        }

        if (y <= 0.0 && time > 2.0) {
            y = 0.0;
            vy = 0.0;
            break;
        }


        Stage& stage = stages[currentStage];

        if(vy<0 && y<=200.0 && !chute){
            chute = true;
            std::cout << "Parachute deployed at time " << time << " seconds.\n";
        }

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
                std::cout << "Stage " << (currentStage+1) << " has run out of fuel at time " << time << " seconds.\n";
                
            }
        } else if (currentStage < stages.size() - 1) {
            stages[currentStage].dry_mass = 0.0; 
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
