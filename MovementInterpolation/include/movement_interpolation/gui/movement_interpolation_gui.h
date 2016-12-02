#ifndef PROJECT_MOVEMENT_INTERPOLATION_GUI_H
#define PROJECT_MOVEMENT_INTERPOLATION_GUI_H

#include <gui/gui.h>

#include <memory>

namespace ifx{
class Renderer;
class EngineGUI;
}

class InterpolationSimulation;
struct InterpolationSimulationCreateParam;

class MovementInterpolationGUI : public ifx::GUI {
public:

    MovementInterpolationGUI(
            GLFWwindow* window,
            std::shared_ptr<ifx::Renderer> renderer,
            std::shared_ptr<InterpolationSimulation> simulation);
    ~MovementInterpolationGUI();

    virtual void Render() override;
private:
    void RenderGUI();

    void RenderSimulationInfo();

    void RenderInterpolationInfo();
    void RenderBeginPosition();
    void RenderEndPosition();

    void RenderBeginEulerAngles();
    void RenderEndEulerAngles();

    void RenderQuaternionInterpolationType();
    void RenderBeginQuaternionAngles();
    void RenderEndQuaternionAngles();

    void InitSimulationCreateParams();

    void TransformEulerToQuaternion();
    void TransformQuaternionToEuler();

    std::shared_ptr<ifx::EngineGUI> engine_gui_;
    std::shared_ptr<InterpolationSimulation> simulation_;

    std::shared_ptr<InterpolationSimulationCreateParam>
            simulation_create_param_;
};


#endif //PROJECT_MOVEMENT_INTERPOLATION_GUI_H
