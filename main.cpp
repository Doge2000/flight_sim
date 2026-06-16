#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <stack>
#include <string> 


const double S_ref = 0.00113; 
const double CL_alpha = 2*M_PI;
const double CD0 = 0.2;
const double k_ind = 0.1;


struct Stage {
    double dry_mass;
    double fuel;
    double thrust;
    double burn_rate;
};

struct PID{
    double kp;
    double ki;
    double kd;
    double integral = 0.0;
    double prev_error = 0.0;

    double update(double error, double dt) {
        integral += error * dt;
        double derivative = (error - prev_error) / dt;
        prev_error = error;
        return kp * error + ki * integral + kd * derivative;
    }
};



double airDensity(double altitude) {
    const double rho0 = 1.225;
    const double H = 8500.0;
    return rho0 * std::exp(-altitude / H);
}

double angleattack(double theta, double vx, double vy) {
    double v = std::sqrt(vx*vx + vy*vy)+1e-9;
    double alpha = std::atan2(vy, vx);
    return theta - alpha;
}

double pitchProgram(double t){
        if(t<2) return M_PI/2;
        if(t<20){
            double frac = (t-2)/18.0;
            return M_PI/2+frac*(20*M_PI/180.0 - M_PI/2);
        }
        return 20*M_PI/180.0;
    }

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

    PID pid = {3.0, 0.0, 1.5};
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
    double x = 1e-6, y = 1e-6;
    double vx = 1e-6, vy = 1e-6;
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

        double Cd_eff = chute?  0.5 : CD0;
        double area_eff = chute? 0.1 : S_ref;
        
        double gamma = std::atan2(vy, vx);

        double gamma_target = pitchProgram(time);
        double error = gamma_target - gamma;

        double update = pid.update(error, dt);
        angle = gamma + update;

        double maxgimbal = 10*M_PI/180.0;
        double delta = angle - gamma;

        if(delta> maxgimbal) angle = gamma + maxgimbal;
        if(delta<-maxgimbal) angle = gamma - maxgimbal;        
      
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
        double alpha = angleattack(angle, vx, vy);
        double q = 0.5 * rho * v * v;
        double CL = CL_alpha * alpha;
        double CD = Cd_eff + k_ind * CL * CL;

        double L = q*CL*area_eff;
        double D = q*CD*area_eff;

        double ex = vx/v;
        double ey = vy/v;

        double Fx_drag = -D*ex;
        double Fy_drag = -D*ey;

        double Fx_lift = -L*ey;
        double Fy_lift = L*ex;

  
        double Fx_thrust = thrust * std::cos(angle);
        double Fy_thrust = thrust * std::sin(angle);

        double Fx = Fx_thrust + Fx_drag + Fx_lift;
        double Fy = Fy_thrust + Fy_drag + Fy_lift - currentmass * g;


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
