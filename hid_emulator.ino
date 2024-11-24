#include <Keyboard.h>
#include <Mouse.h>
#include <math.h>

// Интервалы для выполнения действий
#define KEY_INTERVAL_MIN 150
#define KEY_INTERVAL_MAX 300
#define MOUSE_MOVE_DURATION_MIN 500
#define MOUSE_MOVE_DURATION_MAX 1500

void setup() {
    Keyboard.begin();
    Mouse.begin();
    Serial.begin(9600); // Устанавливаем соединение с Python
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        bool result = processCommand(command);
        Serial.println(result ? "True" : "False");
    }
}

bool handleMouseMoveDirect(String params) {
    int sep = params.indexOf(',');
    if (sep == -1) return false;

    int targetX = params.substring(0, sep).toInt();
    int targetY = params.substring(sep + 1).toInt();

    Mouse.move(targetX, targetY); // Прямое перемещение
    return true;
}

bool processCommand(String command) {
    if (command.startsWith("mouse_move_direct ")) {
        return handleMouseMoveDirect(command.substring(18));
    } else if (command.startsWith("key_down ")) {
        char key = parseKey(command.substring(9));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
        Keyboard.press(key);
        return true;
    } else if (command.startsWith("key_up ")) {
        char key = parseKey(command.substring(7));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
        Keyboard.release(key);
        return true;
    } else if (command.startsWith("key_press ")) {
        char key = parseKey(command.substring(10));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
        Keyboard.press(key);
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
        Keyboard.release(key);
        return true;
    } else if (command.startsWith("key_combo ")) {
        return handleKeyCombo(command.substring(10));
    } else if (command.startsWith("mouse_move ")) {
        return handleMouseMove(command.substring(11));
    } else if (command.startsWith("mouse_down ")) {
        return handleMousePress(command.substring(11), true);
    } else if (command.startsWith("mouse_up ")) {
        return handleMousePress(command.substring(9), false);
    } else if (command.startsWith("mouse_click ")) {
        return handleMouseClick(command.substring(12));
    }
    return false;
}

char parseKey(String keyStr) {
    keyStr.trim();
    if (keyStr == "ctrl") return KEY_LEFT_CTRL;
    if (keyStr == "shift") return KEY_LEFT_SHIFT;
    if (keyStr == "alt") return KEY_LEFT_ALT;
    if (keyStr == "tab") return KEY_TAB;
    if (keyStr == "enter") return KEY_RETURN;
    if (keyStr == "space") return ' ';
    return keyStr[0];
}

bool handleKeyCombo(String combo) {
    int idx = combo.indexOf('+');
    if (idx == -1) return false;

    char modKey = parseKey(combo.substring(0, idx));
    char mainKey = parseKey(combo.substring(idx + 1));

    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.press(modKey);  // Нажимаем модификатор (например, Alt)
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.press(mainKey); // Нажимаем основную клавишу (например, Tab)
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.release(mainKey); // Отпускаем основную клавишу
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.release(modKey);  // Отпускаем модификатор

    return true;
}

bool handleMouseMove(String params) {
    int sep1 = params.indexOf(',');
    if (sep1 == -1) return false;

    int sep2 = params.indexOf(',', sep1 + 1);
    if (sep2 == -1) return false;

    int sep3 = params.indexOf(',', sep2 + 1);
    if (sep3 == -1) return false;

    // Получаем целевые координаты и коэффициенты
    int targetX = params.substring(0, sep1).toInt();
    int targetY = params.substring(sep1 + 1, sep2).toInt();
    float factorX = params.substring(sep2 + 1, sep3).toFloat();
    float factorY = params.substring(sep3 + 1).toFloat();

    // Применяем коэффициенты к целевым координатам
    targetX = round(targetX * factorX);
    targetY = round(targetY * factorY);

    // Проверка: если конечная цель равна (0,0), предотвращаем мгновенное движение
    if (targetX == 0 && targetY == 0) {
        return false;
    }

    unsigned long duration = random(MOUSE_MOVE_DURATION_MIN, MOUSE_MOVE_DURATION_MAX);
    unsigned long startTime = millis();

    int steps = 100; // Количество шагов для плавности движения

    // Текущие координаты
    int currentX = 0, currentY = 0;

    // Вычисляем длину прямой линии между начальной и конечной точками
    float dx = targetX - currentX;
    float dy = targetY - currentY;
    float distance = sqrt(dx * dx + dy * dy);

    // Смещение для опорных точек
    float offset = distance * 0.2; // Отклоняем на 20% от длины прямой линии

    // Единичный вектор, перпендикулярный линии
    float normX = -dy / distance;
    float normY = dx / distance;

    // Опорные точки для кривой Безье с отклонением
    int controlX1 = targetX / 3 + normX * offset;   // Опорная точка на 1/3 пути
    int controlY1 = targetY / 3 + normY * offset;   // Опорная точка на 1/3 пути
    int controlX2 = 2 * targetX / 3 - normX * offset; // Опорная точка на 2/3 пути
    int controlY2 = 2 * targetY / 3 - normY * offset; // Опорная точка на 2/3 пути

    for (int i = 1; i <= steps; i++) {
        // Нормализованный параметр [0..1]
        float t = (float)i / steps;

        // Вычисляем координаты по кривой Безье
        int bezierX = round((1 - t) * (1 - t) * (1 - t) * 0 +
                            3 * (1 - t) * (1 - t) * t * controlX1 +
                            3 * (1 - t) * t * t * controlX2 +
                            t * t * t * targetX);

        int bezierY = round((1 - t) * (1 - t) * (1 - t) * 0 +
                            3 * (1 - t) * (1 - t) * t * controlY1 +
                            3 * (1 - t) * t * t * controlY2 +
                            t * t * t * targetY);

        // Рассчитываем смещение
        int moveX = bezierX - currentX;
        int moveY = bezierY - currentY;

        // Обновляем текущие координаты
        currentX += moveX;
        currentY += moveY;

        // Проверяем границы, включая отрицательные координаты
        if ((moveX > 0 && currentX > targetX) || (moveX < 0 && currentX < targetX)) {
            currentX = targetX;
            moveX = 0;
        }
        if ((moveY > 0 && currentY > targetY) || (moveY < 0 && currentY < targetY)) {
            currentY = targetY;
            moveY = 0;
        }

        // Выполняем перемещение
        Mouse.move(moveX, moveY);

        // Плавная задержка между шагами
        delay(duration / steps);
    }

    // Финальная коррекция с плавным прямолинейным движением (200 мс)
    int finalX = targetX - currentX;
    int finalY = targetY - currentY;

    int correctionSteps = 20; // 20 шагов для 200 мс
    for (int i = 0; i < correctionSteps; i++) {
        int stepX = finalX / correctionSteps;
        int stepY = finalY / correctionSteps;

        Mouse.move(stepX, stepY);

        finalX -= stepX;
        finalY -= stepY;

        delay(200 / correctionSteps);
    }

    return true;
}

bool handleMousePress(String button, bool isDown) {
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    if (button == "left") {
        if (isDown) Mouse.press(MOUSE_LEFT);
        else Mouse.release(MOUSE_LEFT);
    } else if (button == "right") {
        if (isDown) Mouse.press(MOUSE_RIGHT);
        else Mouse.release(MOUSE_RIGHT);
    }
    return true;
}

bool handleMouseClick(String button) {
    return handleMousePress(button, true) && handleMousePress(button, false);
}
