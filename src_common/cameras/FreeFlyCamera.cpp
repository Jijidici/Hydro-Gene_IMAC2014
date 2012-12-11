#include "cameras/FreeFlyCamera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265

namespace hydrogene{

	FreeFlyCamera::FreeFlyCamera(){
		m_Position = glm::vec3(0.f, 0.f, 0.f);
		m_fPhi = PI;
		m_fTheta = 0;

		computeDirectionVectors();
	}

	void FreeFlyCamera::computeDirectionVectors(){
		m_FrontVector = glm::vec3(glm::cos(m_fTheta)*glm::sin(m_fPhi), glm::sin(m_fTheta), glm::cos(m_fTheta)*glm::cos(m_fPhi));
		m_LeftVector = glm::vec3(glm::sin(m_fPhi+(PI/2)), 0.f, glm::cos(m_fPhi+(PI/2)));
		m_UpVector = glm::cross(m_FrontVector, m_LeftVector);
	}

	void FreeFlyCamera::moveLeft(float const t){
		m_Position += t * m_LeftVector;
	}

	void FreeFlyCamera::moveFront(float const t){
		m_Position += t * m_FrontVector;
	}


	void FreeFlyCamera::rotateLeft(float degree){
		m_fPhi = glm::radians(degree);
		computeDirectionVectors();
	}

	void FreeFlyCamera::rotateUp(float degree){
		m_fTheta = glm::radians(degree);
		computeDirectionVectors();
	}

	glm::mat4 FreeFlyCamera::getViewMatrix() const{
		return glm::lookAt(m_Position, m_Position + m_FrontVector, m_UpVector);
	}

}