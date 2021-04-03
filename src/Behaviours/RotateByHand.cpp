#include "Behaviours/RotateByHand.hpp"

DEFINE_CLASS(MapLoader::RotateByHand);

namespace MapLoader
{
    void RotateByHand::Awake()
    {
        midPoint = (Vector3){0.0f, 0.0f, 0.0f};
        angle = 0;
        deltaAngle = 0;
        previousAngle = 0;
        Il2CppObject* go = *il2cpp_utils::RunMethod(this, "get_gameObject");
        
        static std::vector<Il2CppClass*> colliderKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Collider")};
        Il2CppObject* collider = *il2cpp_utils::RunGenericMethod(go, "GetComponent", colliderKlass);
        Bounds bounds = *il2cpp_utils::RunMethod<Bounds>(collider, "get_bounds");
        midPoint = bounds.center;

        /*
        static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
        Il2CppObject* renderer = *il2cpp_utils::RunGenericMethod(go, "GetComponent", rendererKlass);
        if (!renderer) return;

        using GetBounds = function_ptr_t<Bounds, Il2CppObject*>;
        GetBounds get_bounds = il2cpp_functions::resolve_icall("UnityEngine.Renderer::get_bounds_Injected");
        Bounds bounds = *il2cpp_utils::RunMethod<Bounds>(renderer, "get_bounds");
        midPoint = bounds.center;
        */
        // Rigidbody Stuff
        static std::vector<Il2CppClass*> rigidBodyKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Rigidbody")};
        rigidBody = *il2cpp_utils::RunGenericMethod(go, "GetComponent", rigidBodyKlass);
        if (!rigidBody) rigidBody = *il2cpp_utils::RunGenericMethod(go, "AddComponent", rigidBodyKlass);

        il2cpp_utils::RunMethod(rigidBody, "set_useGravity", false);
        il2cpp_utils::RunMethod(rigidBody, "set_angularDrag", 0.8f);
    }

    void RotateByHand::OnTriggerEnter(Il2CppObject* collider)
    {
        if (!CheckIfValid(collider)) return;
        Il2CppObject* transform = *il2cpp_utils::RunMethod(this, "get_transform");
        Vector3 angles = *il2cpp_utils::RunMethod<Vector3>(transform, "get_eulerAngles");
        angle = angles.y;

        startVector = GetVector(collider);
    }

    void RotateByHand::OnTriggerStay(Il2CppObject* collider)
    {
        if (!CheckIfValid(collider)) return;
        Vector3 currentVector = GetVector(collider);
        Il2CppObject* transform = *il2cpp_utils::RunMethod(this, "get_transform");
        Vector3 angles = *il2cpp_utils::RunMethod<Vector3>(transform, "get_eulerAngles");

        float vectorAngle = *il2cpp_utils::RunMethod<float>("UnityEngine", "Vector3", "SignedAngle", currentVector, startVector, (Vector3){0.0f, 1.0f, 0.0f});

        Vector3 moveRotationVector = {angles.x, angle - vectorAngle, angles.y};
        Quaternion moveRotation = *il2cpp_utils::RunMethod<Quaternion>("UnityEngine", "Quaternion", "Euler", moveRotationVector);

        deltaAngle = vectorAngle - previousAngle;
        previousAngle = vectorAngle;
    }

    void RotateByHand::OnTriggerExit(Il2CppObject* collider)
    {
        if (!CheckIfValid(collider)) return;
        float fixedDeltaTime = *il2cpp_utils::RunMethod<float>("UnityEngine", "Time", "get_fixedDeltaTime");
        float angularVelocityFloat = (-deltaAngle / fixedDeltaTime);
        Vector3 angularVelocity = {0.0f, angularVelocityFloat, 0.0f};

        il2cpp_utils::RunMethod(rigidBody, "set_angularVelocity", angularVelocity);
    }

    bool RotateByHand::CheckIfValid(Il2CppObject* collider)
    {
        Il2CppObject* go = *il2cpp_utils::RunMethod(collider, "get_gameObject");
        static std::vector<Il2CppClass*> klass = {il2cpp_utils::GetClassFromName("", "GorillaTriggerColliderHandIndicator")};

        Il2CppObject* handIndicator = *il2cpp_utils::RunGenericMethod(go, "GetComponentInParent", klass);
        if (!handIndicator) return false;
        if (midPoint.x == 0.0f &&
            midPoint.y == 0.0f &&
            midPoint.z == 0.0f)
            return false;
        return true;
    }

    Vector3 RotateByHand::GetVector(Il2CppObject* collider)
    {
        Vector3 closestPoint = *il2cpp_utils::RunMethod<Vector3>(collider, "ClosestPoint", midPoint);
        Vector3 newStart = {-midPoint.x, -midPoint.y, -midPoint.z};
        newStart.x += closestPoint.x;
        newStart.y += closestPoint.y;
        newStart.z += closestPoint.z;

        return newStart;
    }
}