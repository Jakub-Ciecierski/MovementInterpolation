#include <rendering/renderer.h>

#include "movement_interpolation/gui/movement_interpolation_gui.h"
#include <movement_interpolation/interpolation_simulation.h>

#include "engine_gui/engine_gui.h"
#include "engine_gui/factory/engine_gui_factory.h"

#include <gui/imgui/imgui.h>

MovementInterpolationGUI::MovementInterpolationGUI(
        GLFWwindow* window,
        std::shared_ptr<ifx::Renderer> renderer,
        std::shared_ptr<InterpolationSimulation> simulation) :
        ifx::GUI(window),
        simulation_(simulation){
    engine_gui_ = ifx::EngineGUIFactory().CreateEngineGUI(renderer);

    InitSimulationCreateParams();
}

MovementInterpolationGUI::~MovementInterpolationGUI(){}

void MovementInterpolationGUI::Render(){
    NewFrame();

    RenderGUI();
    engine_gui_->Render();

    ImGui::Render();
}

void MovementInterpolationGUI::RenderGUI(){
    ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Interpolation");
    if(ImGui::CollapsingHeader("Simulation"), 1)
        RenderSimulationInfo();
    if(ImGui::CollapsingHeader("Interpolation"), 1)
        RenderInterpolationInfo();
    ImGui::End();
}

void MovementInterpolationGUI::RenderSimulationInfo(){
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::Text("Time: %.2f [s]", simulation_->time_data().total_time);
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    ImGui::ProgressBar(simulation_->time_data().total_time /
                               simulation_->time_data().simulation_length);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(100);
    ImGui::InputFloat("Simulation Length [s]",
                      &simulation_create_param_->simulation_length_s);
    ImGui::PopItemWidth();
    if (ImGui::Button("Reset")) {
        simulation_->Reset(simulation_create_param_);
    }
    ImGui::SameLine();

    std::string play_button_text;
    if (simulation_->IsRunning())
        play_button_text = "Pause";
    else
        play_button_text = "Play";

    if (ImGui::Button(play_button_text.c_str())) {
        simulation_->SetRunning(!simulation_->IsRunning());
    }

    static int frames_count = 15;

    if (ImGui::Button("Render All Frames")) {
        simulation_->SimulateFrames(frames_count, simulation_create_param_);
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    ImGui::InputInt("Frames Count", &frames_count);
    ImGui::PopItemWidth();
}

void MovementInterpolationGUI::RenderInterpolationInfo(){
    if(ImGui::TreeNode("Position")){
        RenderBeginPosition();
        RenderEndPosition();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Euler Angles Degrees [x,y,z] ")){
        RenderBeginEulerAngles();
        RenderEndEulerAngles();
        ImGui::TreePop();

        TransformEulerToQuaternion();
    }
    if(ImGui::TreeNode("Quaternions [w,x,y,z]")){
        RenderQuaternionInterpolationType();
        RenderBeginQuaternionAngles();
        RenderEndQuaternionAngles();
        ImGui::TreePop();

        TransformQuaternionToEuler();
    }

    simulation_->UpdatePosition(simulation_create_param_);
}

void MovementInterpolationGUI::RenderBeginPosition(){
    static float raw[3];

    raw[0] = simulation_create_param_->interpolation_data.position_begin.x;
    raw[1] = simulation_create_param_->interpolation_data.position_begin.y;
    raw[2] = simulation_create_param_->interpolation_data.position_begin.z;

    ImGui::SliderFloat3("Begin Position", raw, -10, 10);

    simulation_create_param_->interpolation_data.position_begin.x = raw[0];
    simulation_create_param_->interpolation_data.position_begin.y = raw[1];
    simulation_create_param_->interpolation_data.position_begin.z = raw[2];
}

void MovementInterpolationGUI::RenderEndPosition(){
    static float raw[3];

    raw[0] = simulation_create_param_->interpolation_data.position_end.x;
    raw[1] = simulation_create_param_->interpolation_data.position_end.y;
    raw[2] = simulation_create_param_->interpolation_data.position_end.z;

    ImGui::SliderFloat3("End Position", raw, -10, 10);

    simulation_create_param_->interpolation_data.position_end.x = raw[0];
    simulation_create_param_->interpolation_data.position_end.y = raw[1];
    simulation_create_param_->interpolation_data.position_end.z = raw[2];
}

void MovementInterpolationGUI::RenderBeginEulerAngles(){
    static float raw[3];

    raw[0] = simulation_create_param_->interpolation_data.euler_begin.x;
    raw[1] = simulation_create_param_->interpolation_data.euler_begin.y;
    raw[2] = simulation_create_param_->interpolation_data.euler_begin.z;

    ImGui::SliderFloat3("Begin Euler Angles", raw, 0, 360);

    simulation_create_param_->interpolation_data.euler_begin.x = raw[0];
    simulation_create_param_->interpolation_data.euler_begin.y = raw[1];
    simulation_create_param_->interpolation_data.euler_begin.z = raw[2];
}

void MovementInterpolationGUI::RenderEndEulerAngles(){
    static float raw[3];

    raw[0] = simulation_create_param_->interpolation_data.euler_end.x;
    raw[1] = simulation_create_param_->interpolation_data.euler_end.y;
    raw[2] = simulation_create_param_->interpolation_data.euler_end.z;

    ImGui::SliderFloat3("End Euler Angles", raw, 0, 360);

    simulation_create_param_->interpolation_data.euler_end.x = raw[0];
    simulation_create_param_->interpolation_data.euler_end.y = raw[1];
    simulation_create_param_->interpolation_data.euler_end.z = raw[2];
}

void MovementInterpolationGUI::RenderQuaternionInterpolationType(){
    static int e = 0;

    if(simulation_create_param_->interpolation_data.interpolation_method ==
       InterpolationMethod::LERP)
        e = 0;
    if(simulation_create_param_->interpolation_data.interpolation_method ==
       InterpolationMethod::SLERP)
        e = 1;

    ImGui::RadioButton("Lerp", &e, 0); ImGui::SameLine();
    ImGui::RadioButton("Slerp", &e, 1);

    if(e == 0){
        simulation_create_param_->interpolation_data.interpolation_method
                = InterpolationMethod::LERP;
    }
    if(e == 1){
        simulation_create_param_->interpolation_data.interpolation_method
                = InterpolationMethod::SLERP;
    }

}

void MovementInterpolationGUI::RenderBeginQuaternionAngles(){
    static float raw[4];

    raw[0] = simulation_create_param_->interpolation_data.quaternion_begin.w;
    raw[1] = simulation_create_param_->interpolation_data.quaternion_begin.x;
    raw[2] = simulation_create_param_->interpolation_data.quaternion_begin.y;
    raw[3] = simulation_create_param_->interpolation_data.quaternion_begin.z;

    ImGui::SliderFloat4("Begin Quaternion", raw, -2, 2);

    simulation_create_param_->interpolation_data.quaternion_begin.w = raw[0];
    simulation_create_param_->interpolation_data.quaternion_begin.x = raw[1];
    simulation_create_param_->interpolation_data.quaternion_begin.y = raw[2];
    simulation_create_param_->interpolation_data.quaternion_begin.z = raw[3];

    simulation_create_param_->interpolation_data.quaternion_begin
            = glm::normalize(simulation_create_param_->interpolation_data.quaternion_begin);
}

void MovementInterpolationGUI::RenderEndQuaternionAngles(){
    static float raw[4];

    raw[0] = simulation_create_param_->interpolation_data.quaternion_end.w;
    raw[1] = simulation_create_param_->interpolation_data.quaternion_end.x;
    raw[2] = simulation_create_param_->interpolation_data.quaternion_end.y;
    raw[3] = simulation_create_param_->interpolation_data.quaternion_end.z;

    ImGui::SliderFloat4("End Quaternion", raw, -2, 2);

    simulation_create_param_->interpolation_data.quaternion_end.w = raw[0];
    simulation_create_param_->interpolation_data.quaternion_end.x = raw[1];
    simulation_create_param_->interpolation_data.quaternion_end.y = raw[2];
    simulation_create_param_->interpolation_data.quaternion_end.z = raw[3];

    simulation_create_param_->interpolation_data.quaternion_end
            = glm::normalize(simulation_create_param_->interpolation_data.quaternion_end);
}

void MovementInterpolationGUI::InitSimulationCreateParams(){
    simulation_create_param_ =
            std::make_shared<InterpolationSimulationCreateParam>();

    simulation_create_param_->interpolation_data.interpolation_method
            = simulation_->interpolation_data().interpolation_method;

    simulation_create_param_->interpolation_data.position_begin
            = simulation_->interpolation_data().position_begin;
    simulation_create_param_->interpolation_data.position_end
            = simulation_->interpolation_data().position_end;

    simulation_create_param_->interpolation_data.euler_begin
            = simulation_->interpolation_data().euler_begin;
    simulation_create_param_->interpolation_data.euler_end
            = simulation_->interpolation_data().euler_end;

    simulation_create_param_->interpolation_data.interpolation_method
            = simulation_->interpolation_data().interpolation_method;
    simulation_create_param_->interpolation_data.quaternion_begin
            = simulation_->interpolation_data().quaternion_begin;
    simulation_create_param_->interpolation_data.quaternion_end
            = simulation_->interpolation_data().quaternion_end;

    simulation_create_param_->simulation_length_s
            = simulation_->time_data().simulation_length;
}

void MovementInterpolationGUI::TransformEulerToQuaternion(){
    simulation_create_param_->interpolation_data.quaternion_begin = glm::quat(
            glm::radians(simulation_create_param_->interpolation_data.euler_begin));
    simulation_create_param_->interpolation_data.quaternion_end = glm::quat(
            glm::radians(simulation_create_param_->interpolation_data.euler_end));

    simulation_create_param_->interpolation_data.quaternion_begin
            = glm::normalize(simulation_create_param_->interpolation_data.quaternion_begin);
    simulation_create_param_->interpolation_data.quaternion_end
            = glm::normalize(simulation_create_param_->interpolation_data
            .quaternion_end);
}

void MovementInterpolationGUI::TransformQuaternionToEuler(){
    /*
    simulation_create_param_->interpolation_data.euler_begin
            = (glm::eulerAngles(
            simulation_create_param_->interpolation_data.quaternion_begin));

    simulation_create_param_->interpolation_data.euler_end
            = (glm::eulerAngles(
            simulation_create_param_->interpolation_data.quaternion_end));
    */

    simulation_create_param_->interpolation_data.euler_begin
            = glm::degrees(glm::eulerAngles(
            simulation_create_param_->interpolation_data.quaternion_begin));

    simulation_create_param_->interpolation_data.euler_end
            = glm::degrees(glm::eulerAngles(
            simulation_create_param_->interpolation_data.quaternion_end));
}