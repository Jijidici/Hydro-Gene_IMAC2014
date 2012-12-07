#include <iostream>
#include <cstdlib>
#include <cmath>
#include <glm/glm.hpp>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include "types.hpp"

static const double HALF_SQRT_3 = sqrt(3)/2.;
static const double TWO_TIERS_SQRT_3 = 2*sqrt(3)/3.;
static const size_t GRID_3D_SIZE = 2;

/******************************************/
/*          FUNCTIONS                     */
/******************************************/

/** MIN/MAX of a Face**/

double getmaxX(Face testedFace){
	double maxX = testedFace.s1->pos.x;
	if(testedFace.s2->pos.x > maxX) maxX = testedFace.s2->pos.x;
	if(testedFace.s3->pos.x > maxX) maxX = testedFace.s3->pos.x;
	
	return maxX;
}

double getminX(Face testedFace){
	double minX = testedFace.s1->pos.x;
	if(testedFace.s2->pos.x < minX) minX = testedFace.s2->pos.x;
	if(testedFace.s3->pos.x < minX) minX = testedFace.s3->pos.x;
	
	return minX;
}

double getmaxY(Face testedFace){
	double maxY = testedFace.s1->pos.y;
	if(testedFace.s2->pos.y > maxY) maxY = testedFace.s2->pos.y;
	if(testedFace.s3->pos.y > maxY) maxY = testedFace.s3->pos.y;
	
	return maxY;
}

double getminY(Face testedFace){
	double minY = testedFace.s1->pos.y;
	if(testedFace.s2->pos.y < minY) minY = testedFace.s2->pos.y;
	if(testedFace.s3->pos.y < minY) minY = testedFace.s3->pos.y;
	
	return minY;
}

double getmaxZ(Face testedFace){
	double maxZ = testedFace.s1->pos.z;
	if(testedFace.s2->pos.z > maxZ) maxZ = testedFace.s2->pos.z;
	if(testedFace.s3->pos.z > maxZ) maxZ = testedFace.s3->pos.z;
	
	return maxZ;
}

double getminZ(Face testedFace){
	double minZ = testedFace.s1->pos.z;
	if(testedFace.s2->pos.z < minZ) minZ = testedFace.s2->pos.z;
	if(testedFace.s3->pos.z < minZ) minZ = testedFace.s3->pos.z;
	
	return minZ;
}

/* GEOMETRICS */
/* Determine if a point is in front of or behind a Face | >0 = in front of | <0 = behind | ==0 = on */
double relativePositionVertexFace(Face f, glm::dvec3 vx){
	glm::dvec3 referentVector = createVector(f.s1->pos, vx);
	return glm::dot(referentVector, f.normal);
}

double relativePositionVertexFace(Plane p, glm::dvec3 vx){
	glm::dvec3 referentVector = createVector(p.s1.pos, vx);
	return glm::dot(referentVector, p.normal);
}

double vecNorm(glm::dvec3 v){
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

/************************/
/******INTERSECTION******/
/************************/
/* Calculation of intersection between a vertex of the face and the voxel */
bool processIntersectionVertexVoxel(Vertex* v, Voxel vox, double threshold){
	/* if the center of the voxel is inside a bounding sphere with a radius of threshold, turn it on */
	glm::dvec3 vertexVoxCenter = createVector(v->pos, vox.c);
	if(vecNorm(vertexVoxCenter) < threshold){
		return true;
	}
	return false;
}

/* Calculation of intersection between an edge of the face and the voxel */
bool processIntersectionEdgeVoxel(Vertex* v1, Vertex* v2, Voxel vox, double threshold){
	/* Projection of the voxel center on the edge */
	glm::dvec3 edgeDir = createVector(v1->pos, v2->pos);
	double edgeLength = vecNorm(edgeDir);
	/* case where the segment is a point */
	if(edgeLength <= 0){
		return false;
	}

	glm::dvec3 edgeDiff = createVector(v1->pos, vox.c);
	float t = glm::dot(edgeDiff, edgeDir);
	/* If the projected isn't on the segment */
	if(t<0. || t>1.){
		return false;
	}
	glm::dvec3 voxCProjected = glm::dvec3(v1->pos.x + t*edgeDir.x, v1->pos.y + t*edgeDir.y, v1->pos.z + t*edgeDir.z);

	/* if the center of the voxel is inside a bounding cylinder with a radius of threshold, turn it on */
	if(vecNorm(createVector(vox.c, voxCProjected)) < threshold){
		return true;
	}
	return false;
}

/* Calculation of intersections between the main plane and the voxel */
bool processIntersectionMainPlaneVoxel(Face testedFace, Voxel currentVoxel){
	/* Calculate the threshold normals and its inverse */
	glm::dvec3 normalizeN = glm::normalize(testedFace.normal);
	glm::dvec3 thresholdNormal = TWO_TIERS_SQRT_3 * currentVoxel.size * normalizeN;
	/* Define the upper and lower plane which surround the triangle face */
	Plane G = createPlane(testedFace.s3->pos + thresholdNormal, testedFace.s2->pos + thresholdNormal, testedFace.s1->pos + thresholdNormal);
	Plane H = createPlane(testedFace.s1->pos - thresholdNormal, testedFace.s2->pos - thresholdNormal, testedFace.s3->pos - thresholdNormal);

	/* Test if the center of the voxel is between the two plane */
	double cRelativityG = relativePositionVertexFace(G, currentVoxel.c);
	double cRelativityH = relativePositionVertexFace(H, currentVoxel.c);

	//std::cout<<"//-> rel G : "<<cRelativityG<<" || rel H : "<<cRelativityH<<std::endl;

	/* If it's the case, the voxel center is in the bounding plane */
	if((cRelativityG <=0 && cRelativityH <=0) || (cRelativityG >=0 && cRelativityH >=0)){
		return true;
	}

	return false;
}

/* Calculate if the voxel center is in the Ei prism */
bool processIntersectionOtherPlanesVoxel(Face testedFace, Voxel currentVoxel){
	/* Define the three perpendicular planes to the trangle Face passing by each edge */
	Plane e1 = createPlane(testedFace.s1->pos, testedFace.s2->pos, testedFace.s2->pos + testedFace.normal);
	Plane e2 = createPlane(testedFace.s2->pos, testedFace.s3->pos, testedFace.s3->pos + testedFace.normal);
	Plane e3 = createPlane(testedFace.s3->pos, testedFace.s1->pos, testedFace.s1->pos + testedFace.normal);

	/* Test if the center of the voxel is on the same side of the tree plan */
	double cRelativityE1 = relativePositionVertexFace(e1, currentVoxel.c);
	double cRelativityE2 = relativePositionVertexFace(e2, currentVoxel.c);
	double cRelativityE3 = relativePositionVertexFace(e3, currentVoxel.c);

	//std::cout<<"//-> rel E1 : "<<cRelativityE1<<" || rel E2 : "<<cRelativityE2<<" || rel E3 : "<<cRelativityE3<<std::endl;

	/* If it's the case, the voxel center is in the Ei prism */
	if((cRelativityE1 <=0 && cRelativityE2 <=0 && cRelativityE3 <=0) || (cRelativityE1 >=0 && cRelativityE2 >=0 && cRelativityE3 >=0)){
		return true;
	}

	return false;
}

/* Main calculation of the intersection between the face and a voxel */
bool processIntersectionPolygonVoxel(Face testedFace, Voxel currentVoxel, uint32_t mode){
	/* vertex Bounding sphere radius and edge bounding cylinder radius */
	double Rc = currentVoxel.size * TWO_TIERS_SQRT_3;

	if((mode == 1)||(mode==0)){
		/* Vertices tests */
		if(processIntersectionVertexVoxel(testedFace.s1, currentVoxel, Rc)){ return true;}
		if(processIntersectionVertexVoxel(testedFace.s2, currentVoxel, Rc)){ return true;}
		if(processIntersectionVertexVoxel(testedFace.s3, currentVoxel, Rc)){ return true;}
	}
	if((mode ==2)||(mode==0)){
		/* Edges tests */
		if(processIntersectionEdgeVoxel(testedFace.s1, testedFace.s2, currentVoxel, Rc)){return true;}
		if(processIntersectionEdgeVoxel(testedFace.s1, testedFace.s3, currentVoxel, Rc)){return true;}
		if(processIntersectionEdgeVoxel(testedFace.s2, testedFace.s3, currentVoxel, Rc)){return true;}
	}
	if((mode == 3)||(mode==0)){
		/* Face test */
		if(processIntersectionMainPlaneVoxel(testedFace, currentVoxel) && processIntersectionOtherPlanesVoxel(testedFace, currentVoxel)){
			return true;
		}
	}
	return false;
}

void printHelp(){
	std::cout << std::endl << "############## HELP ##############" << std::endl;
	std::cout << "usage : ./bin/hg_voxel_maker <number of subdivisions> -<option>" << std::endl << std::endl;
	std::cout << "You can put several options at the same time. Options available :" << std::endl<<std::endl;
	std::cout << "-vtx : testing the intersections regarding the vertices of the faces" << std::endl;	
	std::cout << "-edg : testing the intersections regarding the edges of the faces" << std::endl;	
	std::cout << "-pln : testing the intersections regarding the planes of the faces" << std::endl;	
	std::cout << "#DEFAULT : intersections regarding the vertices, the edges and the planes." << std::endl<<std::endl;	
	std::cout << "-n : saving the faces normals in the voxel_data file" << std::endl;
	std::cout << "-b : saving the bending coefficients in the voxel_data file" << std::endl;
	std::cout << "-g : saving the faces gradients in the voxel_data file" << std::endl;
	std::cout << "-s : saving the surfaces in the voxel_data file" << std::endl;
	std::cout << "-d : saving the drain coefficients in the voxel_data file" << std::endl;
	std::cout << "##################################" << std::endl << std::endl;
}
/*************************************/
/*             MAIN                  */
/*************************************/
int main(int argc, char** argv) {

	/* ************************************************************* */
	/* **************PRE - TRAITEMENT DES VOXELS******************** */
	/* ************************************************************* */
	
	std::cout << std::endl << "############################# INITIALISATION #############################" << std::endl;

	//CHARGEMENT PAGE1.DATA

	size_t test_fic = 0;

	FILE *dataFile = NULL;
	dataFile = fopen("terrain_data/page_1.data", "rb");
	if(NULL == dataFile){
		std::cout << "[!] > Unable to load the file dataFile" << std::endl;
		return EXIT_FAILURE;
	}

	//lecture du nombre de vertex et de faces, puis affichage ds la console
	uint32_t nbVertice = 0, nbFace = 0;	
	test_fic = fread(&nbVertice, sizeof(nbVertice), 1, dataFile);
	test_fic =fread(&nbFace, sizeof(nbFace), 1, dataFile);

	// altitudes min et max de la carte
	double altMin = 0;
	double altMax = 0;
	
	double * positionsData = new double[3*nbVertice];
	test_fic = fread(positionsData, sizeof(double), 3*nbVertice, dataFile); // to read the positions of the vertices
	
	uint32_t * facesData = new uint32_t[3*nbFace];
	test_fic = fread(facesData, sizeof(uint32_t), 3*nbFace, dataFile); // to read the indexes of the vertices which compose each face
	
	fclose(dataFile);

	//CONSTRUCTION OF THE DATA STRUCTURES
	Vertex * tabV = new Vertex[nbVertice];
	
	for(uint32_t n=0;n<nbVertice;++n){ // to create the vertices tab
		tabV[n].pos.x = positionsData[3*n];
		tabV[n].pos.z = positionsData[3*n+1];
		tabV[n].pos.y = positionsData[3*n+2];
		
		// on récupère les altitudes extremes
		if(tabV[n].pos.y > altMax){
			altMax = tabV[n].pos.y;
		}else{
			if(tabV[n].pos.y < altMin){
				altMin = tabV[n].pos.y;
			}
		}
	}

	Face * tabF = new Face[nbFace];
	uint32_t vertexCoordsOffset[3];
	
	// creation of the faces
	for(uint32_t n=0;n<nbFace;++n){
		for(size_t i = 0; i < 3; ++i){
			vertexCoordsOffset[i] = facesData[3*n+i];
		}
		tabF[n].s1 = tabV + vertexCoordsOffset[0] -1;
		tabF[n].s2 = tabV + vertexCoordsOffset[1] -1;
		tabF[n].s3 = tabV + vertexCoordsOffset[2] -1;
		tabF[n].normal = glm::dvec3(0,0,0);
		tabF[n].bending = 0;
		tabF[n].drain = 0;
		tabF[n].gradient = 0;
		tabF[n].surface = 0;
	}

	//VOXELS ARRAY CREATION
	
	//getting the arguments

	uint32_t nbSub = 0;
	int p4Requested = 0;
	int drain = 0;
	int gradient = 0;
	int surface = 0;
	int bending = 0;
	int normal = 0;
	int mode = 0;

	if(argc > 1){
		char** tabArguments = new char*[argc];

		for(int i = 0; i<argc-1; ++i){

			tabArguments[i] = argv[i+1];

			//if a number of subdivisions has been entered
			if(atoi(tabArguments[i])) nbSub = atoi(tabArguments[i]);

			else if(strcmp(tabArguments[i],"help") == 0){
				printHelp();
				return(EXIT_SUCCESS);
			}

			//intersection with vertices
			else if (strcmp(tabArguments[i], "-vtx") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the vertices" << std::endl;
				mode = 1;
			}
			//intersection with edges
			else if (strcmp(tabArguments[i], "-edg") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the edges" << std::endl;
				mode = 2;
			}
			//intersection with planes
			else if (strcmp(tabArguments[i], "-pln") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : intersections regarding the planes" << std::endl;
				mode = 3;
			}

			//normals requested
			else if(strcmp(tabArguments[i],"-n") == 0){
				if(!normal && !p4Requested && mode==0) std::cout << std::endl << "############ REQUESTS ############" << std::endl;
				std::cout << "-> Requested : loading of the normals" << std::endl;
				normal = 1;
			}

			else if ((strcmp(tabArguments[i],"-d") == 0)||(strcmp(tabArguments[i],"-b") == 0)||(strcmp(tabArguments[i],"-g") == 0)||(strcmp(tabArguments[i],"-s") == 0)) {
				if(!normal && !p4Requested && mode==0) std::cout << "############ REQUESTS ############" << std::endl;
				p4Requested = 1;
				//drain coeff requested
				if (strcmp(tabArguments[i],"-d") == 0){
					std::cout << "-> Requested : loading of the drain coefficients" << std::endl;
					drain = 1;
				}
				//bending coeff requested
				else if (strcmp(tabArguments[i],"-b") == 0){
					std::cout << "-> Requested : loading of the bending coefficients" << std::endl;
					bending = 1;
				}
				//surfaces requested
				else if (strcmp(tabArguments[i],"-s") == 0){
					std::cout << "-> Requested : loading of the surfaces" << std::endl;
					surface = 1;
				}
				//gradients requested
				else{
					std::cout << "-> Requested : loading of the gradient coefficients" << std::endl;
					gradient = 1;
				}
			}
			else{
				std::cout << std::endl << "[!] -> Warning : the request \"" << tabArguments[i] << "\" does not exist.";
				printHelp();
			}
		}
		delete[] tabArguments;
		if(p4Requested||normal||mode!=0) std::cout << "##################################" << std::endl << std::endl;
	}else printHelp();

	FILE* normalFile = NULL;
	normalFile = fopen("terrain_data/page_2.data", "rb");
	if(NULL == normalFile){
		std::cout << "[!] -> Unable to load the second data page" << std::endl;
		return EXIT_FAILURE;
	}

	//moving to the beginning of face normals in the file
	test_fic = fseek(normalFile, nbVertice*3*sizeof(double), SEEK_SET);
	if(test_fic != 0){
		std::cout<<"[!] -> Unable to move inside the second data page"<<std::endl;
		return EXIT_FAILURE;
	}

	double * normalData = new double[3*nbFace];
	test_fic = fread(normalData, sizeof(double), 3*nbFace, normalFile);

	for(uint32_t n=0;n<nbFace;++n){
		tabF[n].normal = glm::dvec3(normalData[3*n], normalData[3*n+2], normalData[3*n+1]);
	}

	fclose(normalFile);
	delete[] normalData;

	if(p4Requested){
		int* drainData = new int[nbFace];
		double* otherData = new double[3*nbFace];

		FILE* page4File = NULL;
		page4File = fopen("terrain_data/page_4.data", "rb");
		if(NULL == page4File){
			std::cout << "[!] -> Unable to load the file fourth data page" << std::endl;
			return EXIT_FAILURE;
		}

		//moving to the beginning of the other data in the file
		test_fic = fseek(page4File, nbVertice*(sizeof(int)+3*sizeof(double)), SEEK_SET);
		if(test_fic != 0){
			std::cout<<"[!] -> Unable to move inside the fourth data page"<<std::endl;
			return EXIT_FAILURE;
		}
	
		for(uint32_t n=0;n<nbFace;++n){
			test_fic = fread(&(drainData[n]), sizeof(int), 1, page4File);
			test_fic = fread(&(otherData[3*n]), sizeof(double), 3, page4File);
		}

		if(drain){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].drain = drainData[n];
		}
		if(bending){	
			for(uint32_t n=0;n<nbFace;++n) tabF[n].bending = otherData[n*3];
		}
		if(surface){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].surface = otherData[n*3+1];
		}
		if(gradient){
			for(uint32_t n=0;n<nbFace;++n) tabF[n].gradient = otherData[n*3+2];
		}
	}

	uint32_t test = nbSub;
	uint32_t power = 0;
	
	while(test > 1){
		test = test/2;
		++power;
	}
	
	std::cout << "-> Number of vertices : " << nbVertice << std::endl;
	std::cout << "-> Number of faces : " << nbFace << std::endl;
	std::cout <<"-> Altitude max : "<< altMax<< " - Altitude min : "<< altMin << std::endl << std::endl;

	if(nbSub == 0){
		nbSub = 16;
		std::cout << "[!] -> nbSub = 0, Number of subdivisions initialized to 16" << std::endl;
	}else{
		uint32_t nbLow = pow(2,power);
		uint32_t nbUp = pow(2,power+1);
	
		if(nbSub - nbLow < nbUp - nbSub){
			nbSub = nbLow;
		}else{
			nbSub = nbUp;
		}
		std::cout << "-> Number of subdivisions rounded to the closest power of two" << std::endl;
	}
	
	std::cout << "-> Number of subdivisions : " << nbSub << std::endl;
	std::cout << std::endl << "##########################################################################" << std::endl << std::endl;

	uint32_t nbSubY = nbSub; //number of subdivisions on Y
	size_t const tailleTabVoxel = nbSub*nbSubY*nbSub;
	VoxelData* tabVoxel = NULL;
	tabVoxel = new VoxelData[tailleTabVoxel];
	if(NULL == tabVoxel){
		std::cout<<"[!] -> Allocation failure for tabVoxel"<<std::endl;
		return EXIT_FAILURE;
	}
	
	for(uint32_t n=0;n<tailleTabVoxel;++n){
		tabVoxel[n].nbFaces=0;
		tabVoxel[n].sumNormal = glm::dvec3(0,0,0);
		tabVoxel[n].sumDrain = 0;
		tabVoxel[n].sumGradient = 0;
		tabVoxel[n].sumSurface = 0;
		tabVoxel[n].sumBending = 0;
	}

	double voxelSize = GRID_3D_SIZE/(double)nbSub;
	
	//INTERSECTION PROCESSING
	
	//For each Face
	//#pragma omp parallel for
	for(uint32_t n=0; n<nbFace;++n){

		uint32_t minVoxelX = glm::min(uint32_t(getminX(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);
		uint32_t maxVoxelX = glm::min(uint32_t(getmaxX(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);
		uint32_t minVoxelY = glm::min(uint32_t(getminY(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);
		uint32_t maxVoxelY = glm::min(uint32_t(getmaxY(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);
		uint32_t minVoxelZ = glm::min(uint32_t(getminZ(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);
		uint32_t maxVoxelZ = glm::min(uint32_t(getmaxZ(tabF[n])/voxelSize + nbSub*0.5), nbSub-1);

		//For each cube of the face bounding box
		for(uint32_t k=minVoxelZ; k<=maxVoxelZ; ++k){
			for(uint32_t j=minVoxelY;j<=maxVoxelY; ++j){
				for(uint32_t i=minVoxelX;i<=maxVoxelX;++i){
					Voxel vox = createVoxel(i*voxelSize -1, j*voxelSize -1, k*voxelSize -1, voxelSize);
					if(processIntersectionPolygonVoxel(tabF[n], vox, mode)){
						uint32_t currentIndex = i + nbSub*j + k*nbSub*nbSubY;
						tabVoxel[currentIndex].nbFaces++;
						if(normal) tabVoxel[currentIndex].sumNormal = glm::dvec3(tabVoxel[currentIndex].sumNormal.x + tabF[n].normal.x, tabVoxel[currentIndex].sumNormal.y + tabF[n].normal.y, tabVoxel[currentIndex].sumNormal.z + tabF[n].normal.z);
						if(drain) tabVoxel[currentIndex].sumDrain = tabVoxel[currentIndex].sumDrain + tabF[n].drain;
						if(gradient) tabVoxel[currentIndex].sumGradient = tabVoxel[currentIndex].sumGradient + tabF[n].gradient;
						if(surface) tabVoxel[currentIndex].sumSurface = tabVoxel[currentIndex].sumSurface + tabF[n].surface;
						if(bending) tabVoxel[currentIndex].sumBending = tabVoxel[currentIndex].sumBending + tabF[n].bending;	
					} 						
				}
			}
		}
	}

	//WRITTING THE VOXEL-INTERSECTION FILE
	FILE* voxelFile = NULL;
	voxelFile = fopen("voxels_data/voxel_intersec_1.data", "wb");
	if(NULL == voxelFile){
		std::cout << "[!] > Unable to load the file voxelFile" << std::endl;
		return EXIT_FAILURE;
	}

	test_fic = fwrite(&nbSub, sizeof(uint32_t), 1, voxelFile);
	test_fic = fwrite(tabVoxel, tailleTabVoxel*sizeof(VoxelData), 1, voxelFile);

	fclose(voxelFile);
	
	delete[] positionsData;
	delete[] facesData;

	delete[] tabV;
	delete[] tabF;
	delete[] tabVoxel;

	return EXIT_SUCCESS;
}
