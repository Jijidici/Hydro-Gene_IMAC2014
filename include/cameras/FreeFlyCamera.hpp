#ifndef __FREEFLY_CAMERA_HPP__
#define __FREEFLY_CAMERA_HPP__

#include <glm/glm.hpp>

#include "data_types.hpp"

namespace hydrogene{

class FreeFlyCamera{
	private:
		glm::vec3 m_Position;
		float m_fPhi;
		float m_fTheta;
		glm::vec3 m_FrontVector;
		glm::vec3 m_LeftVector;
		glm::vec3 m_UpVector;
		
		float m_nearDistance, m_farDistance, m_verticalFieldOfView;
		double m_leafSize;

		void computeDirectionVectors();
		void computeFrustumPlanes();

	public:
		
		glm::vec3 m_frustumNearPlanePoint;
		glm::vec3 m_frustumFarPlanePoint;
		glm::vec3 m_frustumTopPlanePoint;
		glm::vec3 m_frustumRightPlanePoint;
		glm::vec3 m_frustumBottomPlanePoint;
		glm::vec3 m_frustumLeftPlanePoint;
		
		glm::vec3 m_frustumNearPlaneNormal;
		glm::vec3 m_frustumFarPlaneNormal;
		glm::vec3 m_frustumTopPlaneNormal;
		glm::vec3 m_frustumRightPlaneNormal;
		glm::vec3 m_frustumBottomPlaneNormal;
		glm::vec3 m_frustumLeftPlaneNormal;
	
	
		FreeFlyCamera(glm::vec3 position, float nearDistance, float farDistance, float verticalFieldOfView, double leafSize);
		//~ FreeFlyCamera();
		void moveLeft(float const t);
		void moveFront(float const t);
		void rotateLeft(float degree);
		void rotateUp(float degree);
		bool leavesFrustum(Leaf& l);
		glm::mat4 getViewMatrix() const;
		glm::vec3 getCameraPosition();
		void setCameraPosition(glm::vec3 position);
		void resetView(float theta, float phi);
};

}

#endif
