#include"utils.h"

// 计算 HDR 贴图相关缓存信息
float* calculateHdrCache(float* HDR, int width, int height) {

    float lumSum = 0.0;

    // 初始化 h 行 w 列的概率密度 pdf 并 统计总亮度
    std::vector<std::vector<float>> pdf(height);
    for (auto& line : pdf) line.resize(width);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float R = HDR[3 * (i * width + j)];
            float G = HDR[3 * (i * width + j) + 1];
            float B = HDR[3 * (i * width + j) + 2];
            float lum = 0.2 * R + 0.7 * G + 0.1 * B;
            pdf[i][j] = lum;
            lumSum += lum;
        }
    }

    // 概率密度归一化
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            pdf[i][j] /= lumSum;

    // 累加每一列得到 x 的边缘概率密度
    std::vector<float> pdf_x_margin;
    pdf_x_margin.resize(width);
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            pdf_x_margin[j] += pdf[i][j];

    // 计算 x 的边缘分布函数
    std::vector<float> cdf_x_margin = pdf_x_margin;
    for (int i = 1; i < width; i++)
        cdf_x_margin[i] += cdf_x_margin[i - 1];

    // 计算 y 在 X=x 下的条件概率密度函数
    std::vector<std::vector<float>> pdf_y_condiciton = pdf;
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            pdf_y_condiciton[i][j] /= pdf_x_margin[j];

    // 计算 y 在 X=x 下的条件概率分布函数
    std::vector<std::vector<float>> cdf_y_condiciton = pdf_y_condiciton;
    for (int j = 0; j < width; j++)
        for (int i = 1; i < height; i++)
            cdf_y_condiciton[i][j] += cdf_y_condiciton[i - 1][j];

    // cdf_y_condiciton 转置为按列存储
    // cdf_y_condiciton[i] 表示 y 在 X=i 下的条件概率分布函数
    std::vector<std::vector<float>> temp = cdf_y_condiciton;
    cdf_y_condiciton = std::vector<std::vector<float>>(width);
    for (auto& line : cdf_y_condiciton) line.resize(height);
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            cdf_y_condiciton[j][i] = temp[i][j];

    // 穷举 xi_1, xi_2 预计算样本 xy
    // sample_x[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 x
    // sample_y[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 y
    // sample_p[i][j] 表示取 (i, j) 点时的概率密度
    std::vector<std::vector<float>> sample_x(height);
    for (auto& line : sample_x) line.resize(width);
    std::vector<std::vector<float>> sample_y(height);
    for (auto& line : sample_y) line.resize(width);
    std::vector<std::vector<float>> sample_p(height);
    for (auto& line : sample_p) line.resize(width);
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            float xi_1 = float(i) / height;
            float xi_2 = float(j) / width;

            // 用 xi_1 在 cdf_x_margin 中 lower bound 得到样本 x
            int x = std::lower_bound(cdf_x_margin.begin(), cdf_x_margin.end(), xi_1) - cdf_x_margin.begin();
            // 用 xi_2 在 X=x 的情况下得到样本 y
            int y = std::lower_bound(cdf_y_condiciton[x].begin(), cdf_y_condiciton[x].end(), xi_2) - cdf_y_condiciton[x].begin();

            // 存储纹理坐标 xy 和 xy 位置对应的概率密度
            sample_x[i][j] = float(x) / width;
            sample_y[i][j] = float(y) / height;
            sample_p[i][j] = pdf[i][j];
        }
    }

    // 整合结果到纹理
    // R,G 通道存储样本 (x,y) 而 B 通道存储 pdf(i, j)
    float* cache = new float[width * height * 3];
    //for (int i = 0; i < width * height * 3; i++) cache[i] = 0.0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cache[3 * (i * width + j)] = sample_x[i][j];        // R
            cache[3 * (i * width + j) + 1] = sample_y[i][j];    // G
            cache[3 * (i * width + j) + 2] = sample_p[i][j];    // B
        }
    }

    return cache;
}
bool cmpx(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3);
    return center1.x < center2.x;
}
bool cmpy(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3);
    return center1.y < center2.y;
}
bool cmpz(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3);
    return center1.z < center2.z;
}
int buildBVHwithSAH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n) {
    if (l > r) return 0;

    nodes.push_back(BVHNode());
    int id = nodes.size() - 1;
    nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].index = 0;
    nodes[id].AA = glm::vec3(1145141919, 1145141919, 1145141919);
    nodes[id].BB = glm::vec3(-1145141919, -1145141919, -1145141919);

    // 计算 AABB
    for (int i = l; i <= r; i++) {
        // 最小点 AA
        float minx = std::min(triangles[i].p1.x, std::min(triangles[i].p2.x, triangles[i].p3.x));
        float miny = std::min(triangles[i].p1.y, std::min(triangles[i].p2.y, triangles[i].p3.y));
        float minz = std::min(triangles[i].p1.z, std::min(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].AA.x = std::min(nodes[id].AA.x, minx);
        nodes[id].AA.y = std::min(nodes[id].AA.y, miny);
        nodes[id].AA.z = std::min(nodes[id].AA.z, minz);
        // 最大点 BB
        float maxx = std::max(triangles[i].p1.x, std::max(triangles[i].p2.x, triangles[i].p3.x));
        float maxy = std::max(triangles[i].p1.y, std::max(triangles[i].p2.y, triangles[i].p3.y));
        float maxz = std::max(triangles[i].p1.z, std::max(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].BB.x = std::max(nodes[id].BB.x, maxx);
        nodes[id].BB.y = std::max(nodes[id].BB.y, maxy);
        nodes[id].BB.z = std::max(nodes[id].BB.z, maxz);
    }

    // 不多于 n 个三角形 返回叶子节点
    if ((r - l + 1) <= n) {
        nodes[id].n = r - l + 1;
        nodes[id].index = l;
        return id;
    }

    // 否则递归建树
    float Cost = INF;
    int Axis = 0;
    int Split = (l + r) / 2;
    for (int axis = 0; axis < 3; axis++) {
        // 分别按 x，y，z 轴排序
        if (axis == 0) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpx);
        if (axis == 1) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpy);
        if (axis == 2) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpz);

        // leftMax[i]: [l, i] 中最大的 xyz 值
        // leftMin[i]: [l, i] 中最小的 xyz 值
        std::vector<glm::vec3> leftMax(r - l + 1, glm::vec3(-INF, -INF, -INF));
        std::vector<glm::vec3> leftMin(r - l + 1, glm::vec3(INF, INF, INF));
        // 计算前缀 注意 i-l 以对齐到下标 0
        for (int i = l; i <= r; i++) {
            Triangle& t = triangles[i];
            int bias = (i == l) ? 0 : 1;  // 第一个元素特殊处理

            leftMax[i - l].x = std::max(leftMax[i - l - bias].x, std::max(t.p1.x, std::max(t.p2.x, t.p3.x)));
            leftMax[i - l].y = std::max(leftMax[i - l - bias].y, std::max(t.p1.y, std::max(t.p2.y, t.p3.y)));
            leftMax[i - l].z = std::max(leftMax[i - l - bias].z, std::max(t.p1.z, std::max(t.p2.z, t.p3.z)));

            leftMin[i - l].x = std::min(leftMin[i - l - bias].x, std::min(t.p1.x, std::min(t.p2.x, t.p3.x)));
            leftMin[i - l].y = std::min(leftMin[i - l - bias].y, std::min(t.p1.y, std::min(t.p2.y, t.p3.y)));
            leftMin[i - l].z = std::min(leftMin[i - l - bias].z, std::min(t.p1.z, std::min(t.p2.z, t.p3.z)));
        }

        // rightMax[i]: [i, r] 中最大的 xyz 值
        // rightMin[i]: [i, r] 中最小的 xyz 值
        std::vector<glm::vec3> rightMax(r - l + 1, glm::vec3(-INF, -INF, -INF));
        std::vector<glm::vec3> rightMin(r - l + 1, glm::vec3(INF, INF, INF));
        // 计算后缀 注意 i-l 以对齐到下标 0
        for (int i = r; i >= l; i--) {
            Triangle& t = triangles[i];
            int bias = (i == r) ? 0 : 1;  // 第一个元素特殊处理

            rightMax[i - l].x = std::max(rightMax[i - l + bias].x, std::max(t.p1.x, std::max(t.p2.x, t.p3.x)));
            rightMax[i - l].y = std::max(rightMax[i - l + bias].y, std::max(t.p1.y, std::max(t.p2.y, t.p3.y)));
            rightMax[i - l].z = std::max(rightMax[i - l + bias].z, std::max(t.p1.z, std::max(t.p2.z, t.p3.z)));

            rightMin[i - l].x = std::min(rightMin[i - l + bias].x, std::min(t.p1.x, std::min(t.p2.x, t.p3.x)));
            rightMin[i - l].y = std::min(rightMin[i - l + bias].y, std::min(t.p1.y, std::min(t.p2.y, t.p3.y)));
            rightMin[i - l].z = std::min(rightMin[i - l + bias].z, std::min(t.p1.z, std::min(t.p2.z, t.p3.z)));
        }

        // 遍历寻找分割
        float cost = INF;
        int split = l;
        for (int i = l; i <= r - 1; i++) {
            float lenx, leny, lenz;
            // 左侧 [l, i]
            glm::vec3 leftAA = leftMin[i - l];
            glm::vec3 leftBB = leftMax[i - l];
            lenx = leftBB.x - leftAA.x;
            leny = leftBB.y - leftAA.y;
            lenz = leftBB.z - leftAA.z;
            float leftS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float leftCost = leftS * (i - l + 1);

            // 右侧 [i+1, r]
            glm::vec3 rightAA = rightMin[i + 1 - l];
            glm::vec3 rightBB = rightMax[i + 1 - l];
            lenx = rightBB.x - rightAA.x;
            leny = rightBB.y - rightAA.y;
            lenz = rightBB.z - rightAA.z;
            float rightS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float rightCost = rightS * (r - i);

            // 记录每个分割的最小答案
            float totalCost = leftCost + rightCost;
            if (totalCost < cost) {
                cost = totalCost;
                split = i;
            }
        }
        // 记录每个轴的最佳答案
        if (cost < Cost) {
            Cost = cost;
            Axis = axis;
            Split = split;
        }
    }

    // 按最佳轴分割
    if (Axis == 0) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpx);
    if (Axis == 1) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpy);
    if (Axis == 2) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpz);

    // 递归
    int left = buildBVHwithSAH(triangles, nodes, l, Split, n);
    int right = buildBVHwithSAH(triangles, nodes, Split + 1, r, n);

    nodes[id].left = left;
    nodes[id].right = right;

    return id;
}
int buildBVHTree(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n) {
    if (l > r) {
        return 0;
    }
    nodes.push_back(BVHNode());
    int id = nodes.size() - 1;
    nodes[id].left = nodes[id].right = nodes[id].index = nodes[id].n = 0;
    nodes[id].AA = glm::vec3(1145141919);
    nodes[id].BB = glm::vec3(-1145141919);
    for (int i = l; i <= r; i++) {

        float minx = std::min(triangles[i].p1.x, std::min(triangles[i].p2.x, triangles[i].p3.x));
        float miny = std::min(triangles[i].p1.y, std::min(triangles[i].p2.y, triangles[i].p3.y));
        float minz = std::min(triangles[i].p1.z, std::min(triangles[i].p2.z, triangles[i].p3.z));

        nodes[id].AA.x = std::min(minx, nodes[id].AA.x);
        nodes[id].AA.y = std::min(miny, nodes[id].AA.y);
        nodes[id].AA.z = std::min(minz, nodes[id].AA.z);

        float maxx = std::max(triangles[i].p1.x, std::max(triangles[i].p2.x, triangles[i].p3.x));
        float maxy = std::max(triangles[i].p1.y, std::max(triangles[i].p2.y, triangles[i].p3.y));
        float maxz = std::max(triangles[i].p1.z, std::max(triangles[i].p2.z, triangles[i].p3.z));

        nodes[id].BB.x = std::max(maxx, nodes[id].BB.x);
        nodes[id].BB.y = std::max(maxy, nodes[id].BB.y);
        nodes[id].BB.z = std::max(maxz, nodes[id].BB.z);



    }
    if ((r - l + 1) <= n) {
        nodes[id].index = l;
        nodes[id].n = r - l + 1;
        return id;
    }
    float lenx = nodes[id].BB.x - nodes[id].AA.x;
    float leny = nodes[id].BB.y - nodes[id].AA.y;
    float lenz = nodes[id].BB.z - nodes[id].AA.z;

    int mid = (l + r) / 2;
    if (lenx >= leny && lenx >= lenz) {
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpx);

    }
    if (leny >= lenx && leny >= lenz) {
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpy);
    }
    if (lenz >= lenx && lenz >= leny) {
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpz);
    }

    nodes[id].left = buildBVHTree(triangles, nodes, l, mid, n);
    nodes[id].right = buildBVHTree(triangles, nodes, mid + 1, r, n);
    return id;

}
void EncodeSSBOTriangle(std::vector<Triangle>& triangles, std::vector<SSBO_Triangle>& triangles_encoded) {
    int n = triangles.size();
    triangles_encoded.resize(n);
    for (int i = 0; i < n; i++) {

        triangles_encoded[i].p1 = glm::vec4(triangles[i].p1, 1.0);
        triangles_encoded[i].p2 = glm::vec4(triangles[i].p2, 1.0);
        triangles_encoded[i].p3 = glm::vec4(triangles[i].p3, 1.0);
        //法线
        triangles_encoded[i].n1 = glm::vec4(triangles[i].n1, 1.0);
        triangles_encoded[i].n2 = glm::vec4(triangles[i].n2, 1.0);
        triangles_encoded[i].n3 = glm::vec4(triangles[i].n3, 1.0);
        //材质
        triangles_encoded[i].emissive = glm::vec4(triangles[i].material.emissive, 1.0);
        triangles_encoded[i].baseColor = triangles[i].material.baseColor;
        triangles_encoded[i].meshId = triangles[i].material.meshId;
        Material m = triangles[i].material;

        triangles_encoded[i].param1 = glm::vec4(m.subsurface, m.metallic, m.specular, m.specularTint);
        triangles_encoded[i].param2 = glm::vec4(m.roughness, m.anisotropic, m.sheen, m.sheenTint);
        triangles_encoded[i].param3 = glm::vec4(m.clearcoat, m.clearcoatGloss, m.IOR, m.transmission);
    }

}
void EncodeSSBOBVH(std::vector<BVHNode>& nodes, std::vector<SSBO_BVH>& nodes_encoded) {
    int n = nodes.size();
    nodes_encoded.resize(n);
    for (int i = 0; i < n; i++) {
        nodes_encoded[i].left = nodes[i].left;
        nodes_encoded[i].right = nodes[i].right;
        nodes_encoded[i].index = nodes[i].index;
        nodes_encoded[i].n = nodes[i].n;
        nodes_encoded[i].AA = glm::vec4(nodes[i].AA, 0.0);
        nodes_encoded[i].BB = glm::vec4(nodes[i].BB, 0.0);
    }
}


// 模型变换矩阵
glm::mat4 getTransformMatrix(glm::vec3 rotateCtrl, glm::vec3 translateCtrl, glm::vec3 scaleCtrl) {
    glm::mat4 unit(    // 单位矩阵
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
        glm::vec4(0, 0, 0, 1)
    );
    glm::mat4 scale = glm::scale(unit, scaleCtrl);
    glm::mat4 translate = glm::translate(unit, translateCtrl);
    glm::mat4 rotate = unit;
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.x), glm::vec3(1, 0, 0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.y), glm::vec3(0, 1, 0));
    rotate = glm::rotate(rotate, glm::radians(rotateCtrl.z), glm::vec3(0, 0, 1));

    glm::mat4 model = translate * rotate * scale;
    return model;
}
