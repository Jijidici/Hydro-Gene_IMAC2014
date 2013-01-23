#include "cameras/FreeFlyCamera.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "data_types.hpp"

#define PI 3.14159265
#define SQRT_3 1.73205081

namespace hydrogene{

	FreeFlyCamera::FreeFlyCamera(float nearDistance, float farDistance, float verticalFieldOfView, double leafSize){
		m_Position = glm::vec3(0.f, 0.3f, 0.f);
		m_fPhi = PI;
		m_fTheta = 0;
		
		m_leafSize = leafSize;
		
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
		glm::vec3 frustumPosition = m_Position - glm::vec3(SQRT_3*m_leafSize*m_FrontVector.x, SQRT_3*m_leafSize*m_FrontVector.y, SQRT_3*m_leafSize*m_FrontVector.z);
		m_frustumNearPlanePoint = frustumPosition + (m_FrontVector*m_nearDistance);
		m_frustumNearPlaneNormal = glm::normalize(frustumPosition - m_frustumNearPlanePoint);
		
		m_frustumFarPlanePoint = frustumPosition + (m_FrontVector*m_farDistance);
		m_frustumFarPlaneNormal = glm::normalize(m_frustumFarPlanePoint - frustumPosition);
		
		float nearHalfHeight = tan(m_verticalFieldOfView/2.) * m_nearDistance;
		
		m_frustumTopPlanePoint = frustumPosition;
		m_frustumTopPlaneNormal = glm::normalize(glm::cross(m_LeftVector, (m_frustumNearPlanePoint + m_UpVector*nearHalfHeight) - frustumPosition));
		
		m_frustumRightPlanePoint = frustumPosition;
		m_frustumRightPlaneNormal = glm::normalize(glm::cross(m_UpVector, (m_frustumNearPlanePoint - m_LeftVector*nearHalfHeight) - frustumPosition));
		
		m_frustumBottomPlanePoint = frustumPosition;
		m_frustumBottomPlaneNormal = glm::normalize(glm::cross((m_frustumNearPlanePoint - m_UpVector*nearHalfHeight) - frustumPosition, m_LeftVector));
		
		m_frustumLeftPlanePoint = frustumPosition;
		m_frustumLeftPlaneNormal = glm::normalize(glm::cross((m_frustumNearPlanePoint + m_LeftVector*nearHalfHeight) - frustumPosition, m_UpVector));
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
	
	bool FreeFlyCamera::leavesFrustum(Leaf& l){
		if(glm::dot(m_frustumNearPlaneNormal, (m_frustumNearPlanePoint - glm::vec3(l.pos.x, l.pos.y, l.pos.z))) < 0.){
			return false;
		}
		if(glm::dot(m_frustumRightPlaneNormal, (glm::vec3(l.pos.x, l.pos.y, l.pos.z) - m_frustumRightPlanePoint)) < 0.){
			return false;
		}
		if(glm::dot(m_frustumLeftPlaneNormal, (glm::vec3(l.pos.x, l.pos.y, l.pos.z) - m_frustumLeftPlanePoint)) < 0.){
			return false;
		}
		if(glm::dot(m_frustumTopPlaneNormal, (glm::vec3(l.pos.x, l.pos.y, l.pos.z) - m_frustumTopPlanePoint)) < 0.){
			return false;
		}
		if(glm::dot(m_frustumBottomPlaneNormal, (glm::vec3(l.pos.x, l.pos.y, l.pos.z) - m_frustumBottomPlanePoint)) < 0.){
			return false;
		}
		
		return true;
	}

	glm::vec3 FreeFlyCamera::getCameraPosition(){
		return m_Position;
	}

}
