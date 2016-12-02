#ifndef PROJECT_INTERPOLATION_SIMULATION_H
#define PROJECT_INTERPOLATION_SIMULATION_H

#include <vr/simulation.h>
#include <math/math_ifx.h>

#include <memory>
#include <vector>

/**
 * http://www.euclideanspace.com/maths/geometry/rotations/conversions/index.htm
 */

namespace ifx{
class Scene;
class RenderObject;
class Renderer;
}

enum class InterpolationMethod {
    LERP, SLERP
};

struct InterpolationData {
    glm::vec3 position_begin;
    glm::vec3 position_end;

    glm::vec3 euler_begin;
    glm::vec3 euler_end;

    glm::quat quaternion_begin;
    glm::quat quaternion_end;
    InterpolationMethod interpolation_method;
};

struct InterpolationSimulationCreateParam{
    float simulation_length_s;

    InterpolationData interpolation_data;
};

// In seconds
struct TimeData{
    float simulation_length;

    float last_time;
    float current_time;

    float total_time;

    float time_since_last_update;
    const float time_delta = 1.0f / 60.0f;
};

struct RenderObjects{
    std::shared_ptr<ifx::RenderObject> render_object_begin_;
    std::shared_ptr<ifx::RenderObject> render_object_end_;

    std::shared_ptr<ifx::RenderObject> render_object_euler_current_;
    std::shared_ptr<ifx::RenderObject> render_object_quaternion_current_;

    std::vector<std::shared_ptr<ifx::RenderObject>> render_objects_euler;
    std::vector<std::shared_ptr<ifx::RenderObject>> render_objects_quaternion;
};

class InterpolationSimulation : public ifx::Simulation {
public:

    InterpolationSimulation(std::shared_ptr<ifx::Scene> scene,
                            std::shared_ptr<ifx::Renderer> renderer,
                            std::shared_ptr<ifx::RenderObject> render_object);
    ~InterpolationSimulation();

    InterpolationData& interpolation_data(){return interpolation_data_;}
    TimeData& time_data(){return time_data_;}

    void SetRunning(bool value) override;

    void UpdatePosition(
            std::shared_ptr<InterpolationSimulationCreateParam> params);
    void Reset(std::shared_ptr<InterpolationSimulationCreateParam> params);
    virtual void Update() override;

    void SimulateFrames(int count,
                        std::shared_ptr<InterpolationSimulationCreateParam> param);
private:
    void Update(double time_elapsed);

    glm::vec3 InterpolatePosition(float t);
    glm::vec3 InterpolateEulerAngles(float t);
    glm::vec3 InterpolateQuaternions(float t);

    void InitScene(std::shared_ptr<ifx::Scene> scene,
                   std::shared_ptr<ifx::RenderObject> render_object);
    void InitParameters();

    InterpolationData interpolation_data_;
    TimeData time_data_;

    std::shared_ptr<ifx::Scene> scene_;
    std::shared_ptr<ifx::Renderer> renderer_;
    RenderObjects render_objects;
};


#endif //PROJECT_INTERPOLATION_SIMULATION_H
