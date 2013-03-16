#include "cameras/TrackBallCamera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tools/MatrixStack.hpp"

namespace hydrogene{

// --> DEFAULT CONSTRUCTOR
TrackBallCamera::TrackBallCamera() : m_fDistance(5), m_fAngleX(60), m_fAngleY(0), m_position(0.f){
}

void TrackBallCamera::moveFront(float distance){
	m_fDistance += distance;
	if(m_fDistance < 0.1f){
		m_fDistance = 0.1f;
	}
}

void TrackBallCamera::rotateLeft(float degrees){
	m_fAngleY = degrees;
}

void TrackBallCamera::rotateUp(float degrees){
	if(degrees >= 5 && degrees <= 175){
		m_fAngleX = degrees;
	}
}

glm::mat4 TrackBallCamera::getViewMatrix(){
	MatrixStack V;
	V.set(glm::lookAt(glm::vec3(0.f,0.f,m_fDistance), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f,1.f,0.f)));
	V.rotate(m_fAngleX, glm::vec3(1.f, 0.f, 0.f));
	V.rotate(m_fAngleY, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 viewMatrix = V.top();
	
	//update cam position
	glm::vec4 camPosition = (glm::inverse(viewMatrix)*glm::vec4(0.f, 0.f, 0.f, 1.f));
	m_position = glm::vec3(camPosition.x, camPosition.y, camPosition.z);
		
	return viewMatrix;
}

void TrackBallCamera::setCamPos(float xDeg, float yDeg, float distance){
	m_fAngleY = yDeg;
	m_fAngleX = xDeg;
	m_fDistance = distance;
}

glm::vec3 TrackBallCamera::getCameraPosition() const{
	return m_position;
}

}
