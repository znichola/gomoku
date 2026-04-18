struct Vector2D {
    long x, y;

    Vector2D(long x, long y);

    Vector2D(const Vector2D& other);
    Vector2D& operator=(const Vector2D& other);
    ~Vector2D();

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(long k) const;

    long dot(const Vector2D& other) const;
    long cross(const Vector2D& other) const;
};
