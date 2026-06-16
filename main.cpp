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

struct AABB {
	Vector3 min;
	Vector3 max;
};

struct Sphere {
	Vector3 center;
	float radius;
};

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

// OBB構造体
struct OBB {
	Vector3 center;
	Vector3 orientations[3]; // 各軸の方向ベクトル（正規化済み）
	Vector3 size;            // 各軸方向の半径
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

Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }

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

// OBBと線分の衝突判定
// セグメントをOBBのローカル空間に変換し、AABBとの判定に帰着させる
bool IsCollision(const Segment& segment, const OBB& obb) {
	// セグメントの始点をOBBのローカル空間へ変換
	Vector3 d = Subtract(segment.origin, obb.center);

	// ローカル空間での始点・方向ベクトルの各軸成分を求める
	float localOrigin[3] = {
	    Dot(d, obb.orientations[0]),
	    Dot(d, obb.orientations[1]),
	    Dot(d, obb.orientations[2]),
	};
	float localDiff[3] = {
	    Dot(segment.diff, obb.orientations[0]),
	    Dot(segment.diff, obb.orientations[1]),
	    Dot(segment.diff, obb.orientations[2]),
	};
	float size[3] = {obb.size.x, obb.size.y, obb.size.z};

	// ローカル空間でAABB [-size, +size] との線分判定（スラブ法）
	float tMin = 0.0f;
	float tMax = 1.0f;

	for (int i = 0; i < 3; i++) {
		if (fabsf(localDiff[i]) < 1e-6f) {
			// 方向がない軸：始点がスラブ外なら非衝突
			if (localOrigin[i] < -size[i] || localOrigin[i] > size[i]) {
				return false;
			}
		} else {
			float t1 = (-size[i] - localOrigin[i]) / localDiff[i];
			float t2 = (size[i] - localOrigin[i]) / localDiff[i];
			if (t1 > t2)
				std::swap(t1, t2);
			tMin = (std::max)(tMin, t1);
			tMax = (std::min)(tMax, t2);
			if (tMin > tMax) {
				return false;
			}
		}
	}

	return tMin <= tMax;
}

// OBBを描画する関数
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// OBBの8頂点をワールド空間で計算
	Vector3 axes[3] = {
	    Scale(obb.orientations[0], obb.size.x),
	    Scale(obb.orientations[1], obb.size.y),
	    Scale(obb.orientations[2], obb.size.z),
	};

	// 8頂点 = center ± axes[0] ± axes[1] ± axes[2]
	Vector3 v[8];
	for (int i = 0; i < 8; i++) {
		v[i] = obb.center;
		v[i] = Add(v[i], Scale(axes[0], (i & 1) ? 1.0f : -1.0f));
		v[i] = Add(v[i], Scale(axes[1], (i & 2) ? 1.0f : -1.0f));
		v[i] = Add(v[i], Scale(axes[2], (i & 4) ? 1.0f : -1.0f));
	}

	// スクリーン座標に変換
	Vector3 s[8];
	for (int i = 0; i < 8; i++) {
		s[i] = Transform(Transform(v[i], viewProjectionMatrix), viewportMatrix);
	}

	// 12本のエッジを描画
	int edges[12][2] = {
	    {0, 1},
        {2, 3},
        {4, 5},
        {6, 7}, // X方向
	    {0, 2},
        {1, 3},
        {4, 6},
        {5, 7}, // Y方向
	    {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}, // Z方向
	};
	for (auto& e : edges) {
		Novice::DrawLine((int)s[e[0]].x, (int)s[e[0]].y, (int)s[e[1]].x, (int)s[e[1]].y, color);
	}
}

// 線分を描画する関数
void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 start = segment.origin;
	Vector3 end = Add(segment.origin, segment.diff);

	Vector3 sScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 eScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

	Novice::DrawLine((int)sScreen.x, (int)sScreen.y, (int)eScreen.x, (int)eScreen.y, color);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	Vector3 cameraTranslate{0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate{0.26f, 0.0f, 0.0f};

	// ImGuiで制御する回転角（度数法）
	Vector3 rotate{0.0f, 0.0f, 0.0f};

	OBB obb{
	    .center = {-1.0f, 0.0f, 0.0f},
	    .orientations =
	        {
	               {1.0f, 0.0f, 0.0f},
	               {0.0f, 1.0f, 0.0f},
	               {0.0f, 0.0f, 1.0f},
	               },
	    .size = {0.5f, 0.5f, 0.5f},
	};

	Segment segment{
	    .origin = {-0.8f, -0.3f, 0.0f},
	    .diff = {0.5f,  0.5f,  0.5f},
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
		ImGui::DragFloat3("obb.center", &obb.center.x, 0.01f);
		ImGui::DragFloat("rotateX", &rotate.x, 1.0f); // 度数法で入力
		ImGui::DragFloat("rotateY", &rotate.y, 1.0f);
		ImGui::DragFloat("rotateZ", &rotate.z, 1.0f);
		ImGui::DragFloat3("obb.orientations[0]", &obb.orientations[0].x, 0.01f);
		ImGui::DragFloat3("obb.orientations[1]", &obb.orientations[1].x, 0.01f);
		ImGui::DragFloat3("obb.orientations[2]", &obb.orientations[2].x, 0.01f);
		ImGui::DragFloat3("obb.size", &obb.size.x, 0.01f);
		ImGui::DragFloat3("segment.origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("segment.diff", &segment.diff.x, 0.01f);
		ImGui::End();

		// 回転角（度）をラジアンに変換してOBBの向きを更新
		float rx = rotate.x * kPi / 180.0f;
		float ry = rotate.y * kPi / 180.0f;
		float rz = rotate.z * kPi / 180.0f;
		Matrix4x4 rotMat = Multiply(Multiply(MakeRotateXMatrix(rx), MakeRotateYMatrix(ry)), MakeRotateZMatrix(rz));

		// 基底ベクトルに回転を適用
		obb.orientations[0] = Normalize({rotMat.m[0][0], rotMat.m[0][1], rotMat.m[0][2]});
		obb.orientations[1] = Normalize({rotMat.m[1][0], rotMat.m[1][1], rotMat.m[1][2]});
		obb.orientations[2] = Normalize({rotMat.m[2][0], rotMat.m[2][1], rotMat.m[2][2]});

		Matrix4x4 cameraRotateMatrix = Multiply(Multiply(MakeRotateXMatrix(cameraRotate.x), MakeRotateYMatrix(cameraRotate.y)), MakeRotateZMatrix(cameraRotate.z));
		Matrix4x4 cameraMatrix = Multiply(cameraRotateMatrix, MakeTranslateMatrix(cameraTranslate));
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		bool collision = IsCollision(segment, obb);
		uint32_t color = collision ? 0xFF0000FF : 0xFFFFFFFF;

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, color);
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix, color);

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