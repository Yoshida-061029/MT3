#include <Novice.h>

const char kWindowTitle[] = "GC2C_08_ヨシダ_ハルキ";

struct Vector2 {
	float x;
	float y;
};

struct Vector3 {
	float x, y, z;
};

struct Matrix4x4 {
	float m[4][4];
};


Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 mat = {};
	mat.m[0][0] = 1.0f;
	mat.m[1][1] = 1.0f;
	mat.m[2][2] = 1.0f;
	mat.m[3][3] = 1.0f;
	mat.m[3][0] = translate.x;
	mat.m[3][1] = translate.y;
	mat.m[3][2] = translate.z;
	return mat;
}


Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 mat = {};
	mat.m[0][0] = scale.x;
	mat.m[1][1] = scale.y;
	mat.m[2][2] = scale.z;
	mat.m[3][3] = 1.0f;
	return mat;
}


Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result = {};
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}


static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) {
	Novice::ScreenPrintf(x, y, "%.2f", v.x);
	Novice::ScreenPrintf(x + kColumnWidth, y, "%.2f", v.y);
	Novice::ScreenPrintf(x + kColumnWidth * 2, y, "%.2f", v.z);
	Novice::ScreenPrintf(x + kColumnWidth * 3, y, "%s", label);
}

void MatrixScreenPrintf(int x, int y, const Matrix4x4& mat, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			Novice::ScreenPrintf(x + col * kColumnWidth, y + (row + 1) * kRowHeight, "%.2f", mat.m[row][col]);
		}
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};


	Vector3 translate{4.1f, 2.6f, 0.8f};
	Vector3 scale{1.5f, 5.2f, 7.3f};

	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	Vector3 point{2.3f, 3.8f, 1.4f};

	Matrix4x4 transformMatrix = {
	    {{1.0f, 2.0f, 3.0f, 4.0f}, {3.0f, 1.0f, 1.0f, 2.0f}, {1.0f, 4.0f, 2.0f, 3.0f}, {2.0f, 2.0f, 1.0f, 3.0f}}
    };

	Vector3 transformed = Transform(point, transformMatrix);

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		VectorScreenPrintf(0, 0, transformed, "transformed");
		MatrixScreenPrintf(0, kRowHeight, translateMatrix, "translateMatrix");
		MatrixScreenPrintf(0, kRowHeight * 7, scaleMatrix, "scaleMatrix");

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