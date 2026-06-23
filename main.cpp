#define _USE_MATH_DEFINES
#include <Novice.h>
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <stdio.h>

const char kWindowTitle[] = "GC2C_08_ヨシダ_ハルキ";
const int kWindowHeight = 720;
const int kWindowWidth = 1280;

constexpr float kPi = 3.14159265358979323846f;

struct Vector3 {
	float x, y, z;
};

struct Matrix4x4 {
	float m[4][4];
};

struct Sphere {
	Vector3 center;
	float radius;
};

Vector3 Subtract(const Vector3& a, const Vector3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vector3 Add(const Vector3& a, const Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float LengthSquared(const Vector3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
float Length(const Vector3& v) { return sqrtf(LengthSquared(v)); }

Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len < 1e-6f)
		return {0, 0, 0};
	return {v.x / len, v.y / len, v.z / len};
}

// 線形補間（2次ベジェ曲線の実装に必要）
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) { return Add(Scale(v1, 1.0f - t), Scale(v2, t)); }

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += a.m[i][k] * b.m[k][j];
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 m = {
	    {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {t.x, t.y, t.z, 1}}
    };
	return m;
}

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

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ) {
	float f = 1.0f / tanf(fovY / 2.0f);
	Matrix4x4 m = {};
	m.m[0][0] = f / aspect;
	m.m[1][1] = f;
	m.m[2][2] = farZ / (farZ - nearZ);
	m.m[2][3] = 1.0f;
	m.m[3][2] = -nearZ * farZ / (farZ - nearZ);
	return m;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minD, float maxD) {
	Matrix4x4 m = {};
	m.m[0][0] = width / 2.0f;
	m.m[1][1] = -height / 2.0f;
	m.m[2][2] = maxD - minD;
	m.m[3][0] = left + width / 2.0f;
	m.m[3][1] = top + height / 2.0f;
	m.m[3][2] = minD;
	m.m[3][3] = 1.0f;
	return m;
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 inv = {};
	float det = 0.0f;
	float mat[4][4];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			mat[i][j] = m.m[i][j];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float sub[3][3];
			int si = 0;
			for (int row = 0; row < 4; row++) {
				if (row == i)
					continue;
				int sj = 0;
				for (int col = 0; col < 4; col++) {
					if (col == j)
						continue;
					sub[si][sj++] = mat[row][col];
				}
				si++;
			}
			float minor =
			    sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) - sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) + sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);
			float cofactor = ((i + j) % 2 == 0 ? 1.0f : -1.0f) * minor;
			inv.m[j][i] = cofactor;
			if (j == 0)
				det += mat[i][0] * cofactor;
		}
	}
	if (fabsf(det) < 1e-6f)
		return inv;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			inv.m[i][j] /= det;
	return inv;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
	float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	if (fabsf(w) > 1e-6f)
		return {x / w, y / w, z / w};
	return {x, y, z};
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const int kSubdivision = 10;
	const float kStep = kGridHalfWidth * 2.0f / kSubdivision;
	for (int i = 0; i <= kSubdivision; i++) {
		float x = -kGridHalfWidth + kStep * i;
		Vector3 s = Transform(Transform({x, 0, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({x, 0, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, color);
	}
	for (int i = 0; i <= kSubdivision; i++) {
		float z = -kGridHalfWidth + kStep * i;
		Vector3 s = Transform(Transform({-kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({kGridHalfWidth, 0, z}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine((int)s.x, (int)s.y, (int)e.x, (int)e.y, color);
	}
}

// 球を緯線・経線のワイヤーフレームで描画する関数（コントロールポイントの可視化用）
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const int kSubdivision = 8;
	const float kLonEvery = 2.0f * kPi / kSubdivision; // 経度方向の分割角度
	const float kLatEvery = kPi / kSubdivision;        // 緯度方向の分割角度（-π/2 ～ π/2）

	auto SpherePoint = [&](float lat, float lon) -> Vector3 {
		return {
		    sphere.center.x + sphere.radius * cosf(lat) * cosf(lon),
		    sphere.center.y + sphere.radius * sinf(lat),
		    sphere.center.z + sphere.radius * cosf(lat) * sinf(lon),
		};
	};

	// 緯線（横方向の円）
	for (int latIndex = 0; latIndex <= kSubdivision; latIndex++) {
		float lat = -kPi / 2.0f + kLatEvery * latIndex;
		for (int lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
			float lon0 = lonIndex * kLonEvery;
			float lon1 = (lonIndex + 1) * kLonEvery;
			Vector3 a = SpherePoint(lat, lon0);
			Vector3 b = SpherePoint(lat, lon1);
			Vector3 sA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 sB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine((int)sA.x, (int)sA.y, (int)sB.x, (int)sB.y, color);
		}
	}
	// 経線（縦方向の円）
	for (int lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
		float lon = lonIndex * kLonEvery;
		for (int latIndex = 0; latIndex < kSubdivision; latIndex++) {
			float lat0 = -kPi / 2.0f + kLatEvery * latIndex;
			float lat1 = -kPi / 2.0f + kLatEvery * (latIndex + 1);
			Vector3 a = SpherePoint(lat0, lon);
			Vector3 b = SpherePoint(lat1, lon);
			Vector3 sA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 sB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine((int)sA.x, (int)sA.y, (int)sB.x, (int)sB.y, color);
		}
	}
}

// 2次ベジェ曲線の描画関数（De Casteljauのアルゴリズムを線分近似で描画）
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const int kSegmentCount = 32; // 曲線を近似する分割数
	Vector3 prevPoint = controlPoint0;

	for (int i = 1; i <= kSegmentCount; i++) {
		float t = float(i) / float(kSegmentCount);

		// p0-p1, p1-p2 をそれぞれ補間し、その結果同士をさらに補間する
		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t);
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t);
		Vector3 point = Lerp(p0p1, p1p2, t);

		Vector3 s0 = Transform(Transform(prevPoint, viewProjectionMatrix), viewportMatrix);
		Vector3 s1 = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine((int)s0.x, (int)s0.y, (int)s1.x, (int)s1.y, color);

		prevPoint = point;
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	// 実装例の初期値（スライド指定）
	Vector3 controlPoints[3] = {
	    {-0.8f, 0.58f, 1.0f },
	    {1.76f, 1.0f,  -0.3f},
	    {0.94f, -0.7f, 2.3f },
	};

	char keys[256] = {0};
	char preKeys[256] = {0};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		ImGui::Begin("Window");
		ImGui::DragFloat3("controlPoints[0]", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("controlPoints[1]", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("controlPoints[2]", &controlPoints[2].x, 0.01f);
		ImGui::End();

		Matrix4x4 cameraRotateMatrix = Multiply(Multiply(MakeRotateXMatrix(cameraRotate.x), MakeRotateYMatrix(cameraRotate.y)), MakeRotateZMatrix(cameraRotate.z));
		Matrix4x4 cameraMatrix = Multiply(cameraRotateMatrix, MakeTranslateMatrix(cameraTranslate));
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawBezier(controlPoints[0], controlPoints[1], controlPoints[2], viewProjectionMatrix, viewportMatrix, 0x0000FFFF);

		// コントロールポイントを黒い球（半径0.01m）で描画
		for (int i = 0; i < 3; i++) {
			Sphere sphere{controlPoints[i], 0.01f};
			DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, 0x000000FF);
		}

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