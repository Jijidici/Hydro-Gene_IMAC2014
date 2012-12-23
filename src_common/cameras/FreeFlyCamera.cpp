#include "cameras/FreeFlyCamera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265

namespace hydrogene{

	FreeFlyCamera::FreeFlyCamera(float nearDistance, float farDistance, float verticalFieldOfView){
		m_Position = glm::vec3(0.f, 0.f, 0.f);
		m_fPhi = PI;
		m_fTheta = 0;
		
		m_nearDistance = nearDistance;
		m_farDistance = farDistance;
		m_verticalFieldOfView = verticalFieldOfView;
		
		m_frustumNearPlanePoint = glm::vec3(0.f, 0.f, 0.f);
		m_frustumFarPlanePoint = glm::vec3(0.f, 0.f, 0.f);
		m_frustumTopPlanePoint = glm::vec3(0.f, 0.f, 0.f);
		m_frustumRightPlanePoint = glm::vec3(0.f, 0.f, 0.f);
		m_frustumBottomPlanePoint = glm::vec3(0.f, 0.f, 0.f);
		m_frustumLeftPlanePoint = glm::vec3(0.f, 0.f, 0.f);

		m_frustumNearPlaneNormal = glm::vec3(0.f, 0.f, 0.f);
		m_frustumFarPlaneNormal = glm::vec3(0.f, 0.f, 0.f);
		m_frustumTopPlaneNormal = glm::vec3(0.f, 0.f, 0.f);
		m_frustumRightPlaneNormal = glm::vec3(0.f, 0.f, 0.f);
		m_frustumBottomPlaneNormal = glm::vec3(0.f, 0.f, 0.f);
		m_frustumLeftPlaneNormal = glm::vec3(0.f, 0.f, 0.f);

		computeDirectionVectors();
		computeFrustumPlanes();
	}

	void FreeFlyCamera::computeDirectionVectors(){
		m_FrontVector = glm::vec3(glm::cos(m_fTheta)*glm::sin(m_fPhi), glm::sin(m_fTheta), glm::cos(m_fTheta)*glm::cos(m_fPhi));
		m_LeftVector = glm::vec3(glm::sin(m_fPhi+(PI/2)), 0.f, glm::cos(m_fPhi+(PI/2)));
		m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
	}
	
	void FreeFlyCamera::computeFrustumPlanes(){
		m_frustumNearPlanePoint = m_Position + (m_FrontVector*m_nearDistance);
		m_frustumFarPlanePoint = m_Position + (m_FrontVector*m_farDistance);
	}

	void FreeFlyCamera::moveLeft(float const t){
		m_Position += t * m_LeftVector;
		computeFrustumPlanes();
	}

	void FreeFlyCamera::moveFront(float const t){
		m_Position += t * m_FrontVector;
		computeFrustumPlanes();
	}


	void FreeFlyCamera::rotateLeft(float degree){
		m_fPhi = glm::radians(degree);
		computeDirectionVectors();
		computeFrustumPlanes();
	}

	void FreeFlyCamera::rotateUp(float degree){
		m_fTheta = glm::radians(degree);
		computeDirectionVectors();
		computeFrustumPlanes();
	}

	glm::mat4 FreeFlyCamera::getViewMatrix() const{
		return glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
	}

}
