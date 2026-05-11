/* Imagen como arreglo dinámico */
struct image {
    int width;
    int height;
    int channels;
    opaque data<5000000>;  /*Máximo 5MB*/
};

/* Resultado */
struct result {
    image img_seq;
    image img_par;
    double time_seq;
    double time_par;
};

program IMG_PROG {
    version IMG_VERS {
        result procesar(image) = 1;
    } = 1;
} = 0x20000002;
