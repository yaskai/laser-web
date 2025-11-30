#include <float.h>
#include <stdint.h>
#include "raylib.h"
#include "raymath.h"

#define CUBE_COUNT  24
#define MAX_BOUNCE	16 

typedef struct {
	Vector3 position;
	Vector3 size;
	Vector3 rotation;
	Model model;
} Cube;

Cube cubes[CUBE_COUNT];

void ReflectLaser(Ray ray, int8_t ignore_cube, uint8_t bounce);

Color colors[] = {
	ORANGE,
	RED,
	GREEN,
	BLUE,
	PURPLE
};

int main () {
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	
	InitWindow(1600, 900, "Laser Reflection");
	SetTargetFPS(60);

	Shader shader = LoadShader("resources/vert.glsl", "resources/norm.glsl");

	for(uint8_t i = 0; i < CUBE_COUNT; i++) {
		Cube *cube = &cubes[i];

		cube->position = (Vector3) {
			GetRandomValue(0, 0),
			GetRandomValue(-20, 20),
			GetRandomValue(-20, 20)
		};

		if(Vector3Distance(Vector3Zero(), cube->position) <= 1) {
			cube->position.y += GetRandomValue(-10, 10);
			cube->position.z += GetRandomValue(-10, 10);
		}

		cube->size = (Vector3) {
			1 + GetRandomValue(0, 30) * 0.1f, 
			1 + GetRandomValue(0, 30) * 0.1f,
			1 + GetRandomValue(0, 30) * 0.1f,
		};

		cube->rotation = (Vector3) {
			//GetRandomValue(0, 360), 
			//GetRandomValue(0, 360), 
			//GetRandomValue(0, 360) 
			GetRandomValue(0, 360),
			0,
			0
		}; 

		cube->rotation = Vector3Scale(cube->rotation, DEG2RAD);

		Mesh cube_mesh = GenMeshCube(1, 1, 1);
		cube->model = LoadModelFromMesh(cube_mesh);

		cubes[i].model.transform = 
			MatrixIdentity();

		cubes[i].model.transform = 
			MatrixMultiply(MatrixScale(cubes[i].size.x, cubes[i].size.y, cubes[i].size.z), cubes[i].model.transform);

		cubes[i].model.transform = 
			MatrixMultiply(MatrixRotateXYZ(cubes[i].rotation), cubes[i].model.transform);

		cubes[i].model.transform =
			MatrixMultiply(MatrixTranslate(cubes[i].position.x, cubes[i].position.y, cubes[i].position.z), cubes[i].model.transform);

		cubes[i].model.materials[0].maps->color = GRAY;
		cubes[i].model.materials[0].shader = shader;
	}

	Camera3D camera = (Camera3D) {
		.up = (Vector3){0, 1, 0},
		.fovy = 60,
		.target = (Vector3){-1, 0, 0},
		.position = (Vector3){-10, 0, 0},
		.projection = CAMERA_PERSPECTIVE 
	};

	Ray ray = (Ray) { .position = Vector3Zero(), .direction = camera.up };
	Vector3 ray_angle = Vector3Zero();
	
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);

		Vector3 cam_forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
		Vector3 cam_right = Vector3CrossProduct(cam_forward, camera.up);

		// Camera movement
		if(IsKeyDown(KEY_W)) {
			camera.position = Vector3Add(camera.position, cam_forward);
			camera.target = Vector3Add(camera.position, cam_forward);
		} else if (IsKeyDown(KEY_S)) {
			camera.position = Vector3Add(camera.position, Vector3Scale(cam_forward, -1));
		}

		if(IsKeyDown(KEY_L)) {
			camera.position = Vector3Add(camera.position, cam_right);
			camera.target = Vector3Add(camera.target, cam_right);
		} else if(IsKeyDown(KEY_H)) {
			camera.position = Vector3Add(camera.position, Vector3Scale(cam_right, -1));
			camera.target = Vector3Add(camera.target, Vector3Scale(cam_right, -1));
		}

		if(IsKeyDown(KEY_J)) {
			camera.position = Vector3Add(camera.position, Vector3Scale(camera.up, -1));
			camera.target = Vector3Add(camera.target, Vector3Scale(camera.up, -1));
		} else if (IsKeyDown(KEY_K)) {
			camera.position = Vector3Add(camera.position, camera.up);
			camera.target = Vector3Add(camera.target, camera.up);
		}

		// Laser direction
		if(IsKeyDown(KEY_A)) {
			ray_angle.x -= 0.5f * GetFrameTime();
		}

		if(IsKeyDown(KEY_D)) {
			ray_angle.x += 0.5f * GetFrameTime();
		}

		Matrix ray_rot = MatrixRotateXYZ((Vector3){ray_angle.x, ray_angle.y, ray_angle.z});
		ray.direction = Vector3Normalize(Vector3Transform((Vector3){0, 0, 1}, ray_rot));

		BeginMode3D(camera);

		BeginBlendMode(BLEND_ADDITIVE);
		BeginShaderMode(shader);

		for(uint8_t i = 0; i < CUBE_COUNT; i++) {
			Matrix mat = cubes[i].model.transform;
			int loc = GetShaderLocation(shader, "mat_model");
			SetShaderValueMatrix(shader, loc, mat);

			DrawMesh(cubes[i].model.meshes[0], cubes[i].model.materials[0], cubes[i].model.transform);
		}

		EndShaderMode();

		ReflectLaser(ray, -1, 0);

		EndBlendMode();

		EndMode3D();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

void ReflectLaser(Ray ray, int8_t ignore_cube, uint8_t bounce) {
	const float max_dist = 9999.0f;

	float closest_dist = max_dist;
	RayCollision closest_hit = (RayCollision){ .hit = false, .distance = max_dist };
	int8_t hit_id = ignore_cube;
	bool hit = false;

	short color_id = (bounce + 1) % 5;
	Color color = ColorAlpha(colors[color_id], 0.85f);

	for(uint8_t i = 0; i < CUBE_COUNT; i++) {
		Cube *cube = &cubes[i];
		if(i == ignore_cube) continue;

		RayCollision hit_info = GetRayCollisionMesh(ray, cube->model.meshes[0], cube->model.transform);
		if(hit_info.hit) {
			float dist = hit_info.distance;
			if(dist < closest_dist) {

				closest_dist = dist;

				closest_hit.point = hit_info.point;
				closest_hit.normal = hit_info.normal;
				closest_hit.distance = hit_info.distance;
				closest_hit.hit = hit_info.hit;

				hit_id = i;
			}
		}		
	}

	hit = closest_hit.hit; 

	if(!hit) {
		DrawCylinderEx(ray.position, Vector3Scale(ray.direction, 999), 0.05f, 0.05f, 8, color);
		return;
	}

	Vector3 norm = Vector3Normalize(closest_hit.normal);

	Vector3 line_start = ray.position;
	Vector3 line_end = closest_hit.point;

	DrawSphere(line_end, 0.1f, color);
	DrawCylinderEx(ray.position, line_end, 0.05f, 0.05f, 8, color);

	if(bounce + 1 > MAX_BOUNCE) return;

	Ray new_ray = (Ray) {
		.position = line_end,
		.direction = Vector3Reflect(ray.direction, closest_hit.normal)
	};	

	ReflectLaser(new_ray, hit_id, ++bounce);	
}

