#include <Novice.h>
#include <cmath>
#include <stdio.h>

const char kWindowTitle[] = "GC2C_08_ヨシダ_ハルキ";

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

void MatrixScreenPrintf(int x, int y, const Matrix4x4& m, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			Novice::ScreenPrintf(x + col * 80, y + 20 + row * 20, "%6.2f", m.m[row][col]);
		}
	}
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	float sx = std::sin(rotate.x), cx = std::cos(rotate.x);
	float sy = std::sin(rotate.y), cy = std::cos(rotate.y);
	float sz = std::sin(rotate.z), cz = std::cos(rotate.z);

	Matrix4x4 result;

	result.m[0][0] = scale.x * (cy * cz);
	result.m[0][1] = scale.x * (cy * sz);
	result.m[0][2] = scale.x * (-sy);
	result.m[0][3] = 0.0f;

	result.m[1][0] = scale.y * (sx * sy * cz - cx * sz);
	result.m[1][1] = scale.y * (sx * sy * sz + cx * cz);
	result.m[1][2] = scale.y * (sx * cy);
	result.m[1][3] = 0.0f;

	result.m[2][0] = scale.z * (cx * sy * cz + sx * sz);
	result.m[2][1] = scale.z * (cx * sy * sz - sx * cz);
	result.m[2][2] = scale.z * (cx * cy);
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Vector3 scale{1.2f, 0.79f, -2.1f};
		Vector3 rotate{0.4f, 1.43f, -0.8f};
		Vector3 translate{2.7f, -4.15f, 1.57f};

		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, worldMatrix, "worldMatrix");

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