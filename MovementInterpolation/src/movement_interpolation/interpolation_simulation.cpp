#include "movement_interpolation/interpolation_simulation.h"

#include <rendering/scene/scene.h>
#include <rendering/renderer.h>
#include <object/render_object.h>
#include <GLFW/glfw3.h>

InterpolationSimulation::InterpolationSimulation(
        std::shared_ptr<ifx::Scene> scene,
        std::shared_ptr<ifx::Renderer> renderer,
        std::shared_ptr<ifx::RenderObject> render_object) :
        scene_(scene),
        renderer_(renderer){
    InitScene(scene_, render_object);
    InitParameters();
    SetRunning(false);
}

InterpolationSimulation::~InterpolationSimulation(){ }

void InterpolationSimulation::SetRunning(bool value){
    ifx::Simulation::SetRunning(value);

    if(value){
        render_objects.render_object_begin_->do_render(false);
        render_objects.render_object_end_->do_render(false);

        render_objects.render_object_euler_current_->do_render(true);
        render_objects.render_object_quaternion_current_->do_render(true);
    }
    else{
        render_objects.render_object_begin_->do_render(true);
        render_objects.render_object_end_->do_render(true);
    }
}

void InterpolationSimulation::UpdatePosition(
        std::shared_ptr<InterpolationSimulationCreateParam> params){
    render_objects.render_object_begin_->moveTo(
            params->interpolation_data.position_begin);
    render_objects.render_object_end_->moveTo(
            params->interpolation_data.position_end);

    render_objects.render_object_begin_->rotateTo(
            params->interpolation_data.euler_begin);
    render_objects.render_object_end_->rotateTo(
            params->interpolation_data.euler_end);
}

void InterpolationSimulation::Reset(
        std::shared_ptr<InterpolationSimulationCreateParam> param){
    SetRunning(false);
    time_data_.simulation_length = param->simulation_length_s;
    interpolation_data_.interpolation_method
            = param->interpolation_data.interpolation_method;

    interpolation_data_.position_begin = param->interpolation_data.position_begin;
    interpolation_data_.position_end = param->interpolation_data.position_end;
    interpolation_data_.euler_begin = param->interpolation_data.euler_begin;
    interpolation_data_.euler_end = param->interpolation_data.euler_end;

    interpolation_data_.quaternion_begin
            = param->interpolation_data.quaternion_begin;
    interpolation_data_.quaternion_end
            = param->interpolation_data.quaternion_end;

    time_data_.total_time = 0.0f;
    time_data_.current_time = 0.0f;
    time_data_.time_since_last_update = 0.0f;
    time_data_.last_time = glfwGetTime();

    render_objects.render_object_euler_current_->moveTo(interpolation_data_.position_begin);
    render_objects.render_object_euler_current_->rotateTo(interpolation_data_.euler_begin);

    render_objects.render_object_quaternion_current_->moveTo(interpolation_data_.position_begin);
    render_objects.render_object_quaternion_current_->rotateTo(interpolation_data_.euler_begin);

    render_objects.render_object_euler_current_->do_render(false);
    render_objects.render_object_quaternion_current_->do_render(false);

    for(auto& object : render_objects.render_objects_euler){
        scene_->DeleteRenderObject(object.get());
    }
    render_objects.render_objects_euler.clear();
    for(auto& object : render_objects.render_objects_quaternion){
        scene_->DeleteRenderObject(object.get());
    }
    render_objects.render_objects_quaternion.clear();
}

void InterpolationSimulation::Update(){
    time_data_.current_time = glfwGetTime();
    if(!running_){
        time_data_.last_time = time_data_.current_time;
        return;
    }
    double elapsed = time_data_.current_time - time_data_.last_time;
    time_data_.time_since_last_update += elapsed;
    time_data_.total_time += elapsed;
    time_data_.last_time = time_data_.current_time;

    if(time_data_.total_time > time_data_.simulation_length){
        SetRunning(false);
        return;
    }
    if(time_data_.time_since_last_update >= time_data_.time_delta){
        time_data_.time_since_last_update = 0.0f;
        Update(elapsed);
    }
}

void InterpolationSimulation::SimulateFrames(
        int count,
        std::shared_ptr<InterpolationSimulationCreateParam> param){
    Reset(param);
    SetRunning(false);
    time_data_.total_time = time_data_.simulation_length;
    for(int i = 0 ; i < count; i++){
        float t = (float) i / (float) count;
        glm::vec3 pos = InterpolatePosition(t);
        glm::vec3 euler = InterpolateEulerAngles(t);
        glm::vec3 quat_euler = InterpolateQuaternions(t);

        auto euler_object
                = std::shared_ptr<ifx::RenderObject>(
                        new ifx::RenderObject(
                                *(render_objects.render_object_begin_.get())));
        euler_object->id(ObjectID(-1));
        euler_object->moveTo(pos);
        euler_object->rotateTo(euler);
        auto quat_object
                = std::shared_ptr<ifx::RenderObject>(
                        new ifx::RenderObject(
                                *(render_objects.render_object_begin_.get())));
        quat_object->id(ObjectID(-2));
        quat_object->moveTo(pos);
        quat_object->rotateTo(quat_euler);

        render_objects.render_objects_euler.push_back(euler_object);
        render_objects.render_objects_quaternion.push_back(quat_object);
        scene_->AddRenderObject(euler_object);
        scene_->AddRenderObject(quat_object);
    }
}

void InterpolationSimulation::Update(double time_elapsed){
    float t = time_data_.total_time / time_data_.simulation_length;

    glm::vec3 pos = InterpolatePosition(t);
    render_objects.render_object_euler_current_->moveTo(pos);
    render_objects.render_object_quaternion_current_->moveTo(pos);

    render_objects.render_object_euler_current_->rotateTo(
            InterpolateEulerAngles(t));
    render_objects.render_object_quaternion_current_->rotateTo(
            InterpolateQuaternions(t));
}

glm::vec3 InterpolationSimulation::InterpolatePosition(float t){
    glm::vec3 direction = interpolation_data_.position_end -
                          interpolation_data_.position_begin;
    glm::vec3 interpolated_position
            = interpolation_data_.position_begin + direction * t;

    return interpolated_position;
}

glm::vec3 InterpolationSimulation::InterpolateEulerAngles(float t){
    glm::vec3 diff = interpolation_data_.euler_end - interpolation_data_
            .euler_begin;

    glm::vec3 interpolated_angles = interpolation_data_ .euler_begin + diff * t;

    return interpolated_angles;
}

glm::vec3 InterpolationSimulation::InterpolateQuaternions(float t){
    glm::quat q;
    if(interpolation_data_.interpolation_method == InterpolationMethod::LERP){
        q = glm::lerp(interpolation_data_.quaternion_begin,
                      interpolation_data_.quaternion_end, t);
    }
    else if(interpolation_data_.interpolation_method ==
            InterpolationMethod::SLERP){
        q = glm::slerp(interpolation_data_.quaternion_begin,
                       interpolation_data_.quaternion_end, t);
    }
    q = glm::normalize(q);
    glm::vec3 euler = glm::degrees(glm::eulerAngles(q));

    return euler;
}

void InterpolationSimulation::InitScene(
        std::shared_ptr<ifx::Scene> scene,
        std::shared_ptr<ifx::RenderObject> render_object){
    render_objects.render_object_begin_ = std::shared_ptr<ifx::RenderObject>(
            new ifx::RenderObject(*(render_object.get())));
    render_objects.render_object_end_ = std::shared_ptr<ifx::RenderObject>(
            new ifx::RenderObject(*(render_object.get())));

    render_objects.render_object_euler_current_
            = std::shared_ptr<ifx::RenderObject>(
            new ifx::RenderObject(*(render_object.get())));
    render_objects.render_object_euler_current_->id(ObjectID(-1));
    render_objects.render_object_quaternion_current_
            = std::shared_ptr<ifx::RenderObject>(
            new ifx::RenderObject(*(render_object.get())));
    render_objects.render_object_quaternion_current_->id(ObjectID(-2));

    scene->AddRenderObject(render_objects.render_object_begin_);
    scene->AddRenderObject(render_objects.render_object_end_);
    scene->AddRenderObject(render_objects.render_object_euler_current_);
    scene->AddRenderObject(render_objects.render_object_quaternion_current_);
}

void InterpolationSimulation::InitParameters(){
    std::shared_ptr<InterpolationSimulationCreateParam> param =
            std::make_shared<InterpolationSimulationCreateParam>();
    param->simulation_length_s = 4.0;
    param->interpolation_data.position_begin = glm::vec3(0,0,0);
    param->interpolation_data.position_end = glm::vec3(2,2,2);

    param->interpolation_data.euler_begin = glm::vec3(0,0,0);
    param->interpolation_data.euler_end = glm::vec3(67,20,270);

    param->interpolation_data.quaternion_begin = glm::quat(
            glm::radians(param->interpolation_data.euler_begin));
    param->interpolation_data.quaternion_end = glm::quat(
            glm::radians(param->interpolation_data.euler_end));

    param->interpolation_data.interpolation_method
            = InterpolationMethod::SLERP;
    param->interpolation_data.quaternion_begin
            = glm::normalize (param->interpolation_data.quaternion_begin);
    param->interpolation_data.quaternion_end
            = glm::normalize(param->interpolation_data.quaternion_end);

    Reset(param);
}
