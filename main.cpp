#include <Novice.h>

const char kWindowTitle[] = "GC2C_08_ヨシダ_ハルキ";

struct Vector2 {
	float x;
	float y;
};

struct Matrix4x4 {
	float m[4][4];
};

static const int kRowHeight = 20;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix) {
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(x + column * kColumnWidth, y + row * kRowHeight, "%6.02f", matrix.m[row][column]);
		}
	}
}

Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m1.m[row][col] + m2.m[row][col];
		}
	}
	return result;
}

Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m1.m[row][col] - m2.m[row][col];
		}
	}
	return result;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			for (int k = 0; k < 4; ++k) {
				result.m[row][col] += m1.m[row][k] * m2.m[k][col];
			}
		}
	}
	return result;
}

static float Det3x3(float a00, float a01, float a02, float a10, float a11, float a12, float a20, float a21, float a22) {
	return a00 * (a11 * a22 - a12 * a21) - a01 * (a10 * a22 - a12 * a20) + a02 * (a10 * a21 - a11 * a20);
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	const auto& a = m.m;

	auto cols = [](int skip, int out[3]) {
		int n = 0;
		for (int i = 0; i < 4; i++)
			if (i != skip)
				out[n++] = i;
	};

	Matrix4x4 result = {};
	float det = 0;
	int c[3];

	for (int j = 0; j < 4; j++) {
		cols(j, c);
		float d = Det3x3(a[1][c[0]], a[1][c[1]], a[1][c[2]], a[2][c[0]], a[2][c[1]], a[2][c[2]], a[3][c[0]], a[3][c[1]], a[3][c[2]]);
		det += (j % 2 == 0 ? 1 : -1) * a[0][j] * d;
	}

	float inv = 1.0f / det;
	int r[3];

	for (int i = 0; i < 4; i++) {
		cols(i, r);
		for (int j = 0; j < 4; j++) {
			cols(j, c);
			float d = Det3x3(a[r[0]][c[0]], a[r[0]][c[1]], a[r[0]][c[2]], a[r[1]][c[0]], a[r[1]][c[1]], a[r[1]][c[2]], a[r[2]][c[0]], a[r[2]][c[1]], a[r[2]][c[2]]);
			result.m[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * inv * d;
		}
	}

	return result;
}

Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m.m[col][row];
		}
	}
	return result;
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f;
	}
	return result;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Matrix4x4 m1 = {3.2f, 0.7f, 9.6f, 4.4f, 5.5f, 1.3f, 7.8f, 2.1f, 6.9f, 8.0f, 2.6f, 1.0f, 0.5f, 7.2f, 5.1f, 3.3f};
	Matrix4x4 m2 = {4.1f, 6.5f, 3.3f, 2.2f, 8.8f, 0.6f, 9.9f, 7.7f, 1.1f, 5.5f, 6.6f, 0.0f, 3.3f, 9.9f, 8.8f, 2.2f};

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
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

		Novice::ScreenPrintf(0, 0, "Add");
		MatrixScreenPrintf(0, 20, Add(m1, m2));

		Novice::ScreenPrintf(0, 100, "Subtract");
		MatrixScreenPrintf(0, 120, Subtract(m1, m2));

		Novice::ScreenPrintf(0, 200, "Multiply");
		MatrixScreenPrintf(0, 220, Multiply(m1, m2));

		Novice::ScreenPrintf(0, 300, "inverseM1");
		MatrixScreenPrintf(0, 316, Inverse(m1));

		Novice::ScreenPrintf(0, 396, "inverseM2");
		MatrixScreenPrintf(0, 412, Inverse(m2));

		Novice::ScreenPrintf(270, 0, "transposeM1");
		MatrixScreenPrintf(270, 20, Transpose(m1));

		Novice::ScreenPrintf(270, 100, "transposeM2");
		MatrixScreenPrintf(270, 120, Transpose(m2));

		Novice::ScreenPrintf(270, 200, "identity");
		MatrixScreenPrintf(270, 220, MakeIdentity4x4());

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}