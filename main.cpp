#define _USE_MATH_DEFINES
#include <Novice.h>
#include <cmath>
#include <imgui.h>

const char kWindowTitle[] = "GC2C_08_ヨシダ_ハルキ";
const int kWindowHeight = 720;
const int kWindowWidth = 1280;

constexpr float kPi = 3.14159265358979323846f;

struct Vector3 {
	float x, y, z;

	// ===== 演算子オーバーロード =====
	Vector3 operator+(const Vector3& other) const { return {x + other.x, y + other.y, z + other.z}; }
	Vector3 operator-(const Vector3& other) const { return {x - other.x, y - other.y, z - other.z}; }
	Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
	Vector3 operator-() const { return {-x, -y, -z}; }

	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	Vector3& operator*=(float s) {
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
};

// スカラー * ベクトルの形（2.4f * a のような書き方）にも対応させるためのフリー関数
Vector3 operator*(float s, const Vector3& v) { return v * s; }

struct Matrix4x4 {
	float m[4][4];

	// ===== 演算子オーバーロード =====
	Matrix4x4 operator+(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result.m[i][j] = m[i][j] + other.m[i][j];
		return result;
	}
	Matrix4x4 operator-(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result.m[i][j] = m[i][j] - other.m[i][j];
		return result;
	}
	Matrix4x4 operator*(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					result.m[i][j] += m[i][k] * other.m[k][j];
		return result;
	}
};

Matrix4x4 MakeRotateXMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{1, 0, 0, 0}, {0, c, s, 0}, {0, -s, c, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{c, 0, -s, 0}, {0, 1, 0, 0}, {s, 0, c, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakeRotateZMatrix(float angle) {
	float c = cosf(angle), s = sinf(angle);
	Matrix4x4 m = {
	    {{c, s, 0, 0}, {-s, c, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
    };
	return m;
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 m = {};
	for (int i = 0; i < 4; i++)
		m.m[i][i] = 1.0f;
	return m;
}

Matrix4x4 MakeScaleMatrix(const Vector3& s) {
	Matrix4x4 m = MakeIdentity4x4();
	m.m[0][0] = s.x;
	m.m[1][1] = s.y;
	m.m[2][2] = s.z;
	return m;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 m = MakeIdentity4x4();
	m.m[3][0] = t.x;
	m.m[3][1] = t.y;
	m.m[3][2] = t.z;
	return m;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 s = MakeScaleMatrix(scale);
	Matrix4x4 rx = MakeRotateXMatrix(rotate.x);
	Matrix4x4 ry = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rz = MakeRotateZMatrix(rotate.z);
	Matrix4x4 r = rx * ry * rz;
	Matrix4x4 t = MakeTranslateMatrix(translate);
	return s * r * t;
}

Matrix4x4 Inverse(const Matrix4x4& src) {
	float a[4][8];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			a[i][j] = src.m[i][j];
		for (int j = 0; j < 4; j++)
			a[i][4 + j] = (i == j) ? 1.0f : 0.0f;
	}

	for (int col = 0; col < 4; col++) {
		int pivot = col;
		float maxAbs = fabsf(a[col][col]);
		for (int row = col + 1; row < 4; row++) {
			if (fabsf(a[row][col]) > maxAbs) {
				maxAbs = fabsf(a[row][col]);
				pivot = row;
			}
		}
		if (pivot != col) {
			for (int k = 0; k < 8; k++)
				std::swap(a[col][k], a[pivot][k]);
		}

		float divisor = a[col][col];
		if (divisor != 0.0f) {
			for (int k = 0; k < 8; k++)
				a[col][k] /= divisor;
		}

		for (int row = 0; row < 4; row++) {
			if (row == col)
				continue;
			float factor = a[row][col];
			for (int k = 0; k < 8; k++)
				a[row][k] -= factor * a[col][k];
		}
	}

	Matrix4x4 result{};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			result.m[i][j] = a[i][4 + j];
	return result;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + 1.0f * m.m[3][0];
	float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + 1.0f * m.m[3][1];
	float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + 1.0f * m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + 1.0f * m.m[3][3];
	if (w != 0.0f) {
		x /= w;
		y /= w;
		z /= w;
	}
	return {x, y, z};
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	float cot = 1.0f / tanf(fovY / 2.0f);
	Matrix4x4 m = {};
	m.m[0][0] = (1.0f / aspectRatio) * cot;
	m.m[1][1] = cot;
	m.m[2][2] = farClip / (farClip - nearClip);
	m.m[2][3] = 1.0f;
	m.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return m;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 m = MakeIdentity4x4();
	m.m[0][0] = width / 2.0f;
	m.m[1][1] = -height / 2.0f;
	m.m[2][2] = maxDepth - minDepth;
	m.m[3][0] = left + width / 2.0f;
	m.m[3][1] = top + height / 2.0f;
	m.m[3][2] = minDepth;
	return m;
}

float Length(const Vector3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len == 0.0f)
		return {0, 0, 0};
	return {v.x / len, v.y / len, v.z / len};
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + xIndex * kGridEvery;
		Vector3 start{x, 0.0f, -kGridHalfWidth};
		Vector3 end{x, 0.0f, kGridHalfWidth};

		Vector3 ndcStart = Transform(start, viewProjectionMatrix);
		Vector3 ndcEnd = Transform(end, viewProjectionMatrix);
		Vector3 screenStart = Transform(ndcStart, viewportMatrix);
		Vector3 screenEnd = Transform(ndcEnd, viewportMatrix);

		uint32_t color = (fabsf(x) < 1e-4f) ? 0x000000FFu : 0xAAAAAAFFu;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + zIndex * kGridEvery;
		Vector3 start{-kGridHalfWidth, 0.0f, z};
		Vector3 end{kGridHalfWidth, 0.0f, z};

		Vector3 ndcStart = Transform(start, viewProjectionMatrix);
		Vector3 ndcEnd = Transform(end, viewProjectionMatrix);
		Vector3 screenStart = Transform(ndcStart, viewportMatrix);
		Vector3 screenEnd = Transform(ndcEnd, viewportMatrix);

		uint32_t color = (fabsf(z) < 1e-4f) ? 0x000000FFu : 0xAAAAAAFFu;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

struct Sphere {
	Vector3 center;
	float radius;
};

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 12;
	const float kLatEvery = kPi / float(kSubdivision);
	const float kLonEvery = 2.0f * kPi / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -kPi / 2.0f + kLatEvery * latIndex;
		float latNext = lat + kLatEvery;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			float lonNext = lon + kLonEvery;

			Vector3 a{sphere.center.x + sphere.radius * cosf(lat) * cosf(lon), sphere.center.y + sphere.radius * sinf(lat), sphere.center.z + sphere.radius * cosf(lat) * sinf(lon)};
			Vector3 b{sphere.center.x + sphere.radius * cosf(latNext) * cosf(lon), sphere.center.y + sphere.radius * sinf(latNext), sphere.center.z + sphere.radius * cosf(latNext) * sinf(lon)};
			Vector3 c{sphere.center.x + sphere.radius * cosf(lat) * cosf(lonNext), sphere.center.y + sphere.radius * sinf(lat), sphere.center.z + sphere.radius * cosf(lat) * sinf(lonNext)};

			Vector3 screenA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 screenB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 screenC = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}

struct Spring {
	Vector3 anchor;
	float naturalLength;
	float stiffness;
	float dampingCoefficient;
};

struct Ball {
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float mass;
	float radius;
	uint32_t color;
};

void UpdateSpring(Spring& spring, Ball& ball, float deltaTime) {
	Vector3 diff = ball.position - spring.anchor;
	float length = Length(diff);
	if (length == 0.0f)
		return;

	Vector3 direction = Normalize(diff);
	Vector3 restPosition = spring.anchor + direction * spring.naturalLength;
	Vector3 displacement = ball.position - restPosition;

	Vector3 restoringForce = displacement * (-spring.stiffness);
	Vector3 dampingForce = ball.velocity * (-spring.dampingCoefficient);
	Vector3 force = restoringForce + dampingForce;

	ball.acceleration = force * (1.0f / ball.mass);
	ball.velocity += ball.acceleration * deltaTime;
	ball.position += ball.velocity * deltaTime;
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// ===== 演算子オーバーロード確認用 =====
	Vector3 a{0.2f, 1.0f, 0.0f};
	Vector3 b{2.4f, 3.1f, 1.2f};
	Vector3 c = a + b;
	Vector3 d = a - b;
	Vector3 e = a * 2.4f;
	Vector3 rotate{0.4f, 1.43f, -0.8f};
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix; // operator* の確認

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	Spring spring{};
	spring.anchor = {0.0f, 0.0f, 0.0f};
	spring.naturalLength = 1.0f;
	spring.stiffness = 100.0f;
	spring.dampingCoefficient = 2.0f;

	Ball ball{};
	ball.position = {1.2f, 0.0f, 0.0f};
	ball.mass = 2.0f;
	ball.radius = 0.05f;
	ball.color = 0x0000FFFFu;

	bool isSimulating = false;

	char keys[256] = {0};
	char preKeys[256] = {0};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraMatrix = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (isSimulating) {
			const float deltaTime = 1.0f / 60.0f;
			UpdateSpring(spring, ball, deltaTime);
		}

		ImGui::Begin("Window");
		if (ImGui::Button("Start")) {
			isSimulating = true;
		}
		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		{
			Vector3 screenAnchor = Transform(Transform(spring.anchor, viewProjectionMatrix), viewportMatrix);
			Vector3 screenBall = Transform(Transform(ball.position, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine(int(screenAnchor.x), int(screenAnchor.y), int(screenBall.x), int(screenBall.y), 0x000000FFu);
		}

		DrawSphere({ball.position, ball.radius}, viewProjectionMatrix, viewportMatrix, ball.color);

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}