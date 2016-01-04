<?php
/**
 * Función que crea un tipo de acción para el bulk operacion llamando una función
 */
function conectorSegeco1_action_info() { 
    return array(
        'conectorSegeco1_sincronizarViales' => array(
            'type' => 'entity',
            'label' => t('Sincronizar viales, añadir a la cola'),
            'configurable' => FALSE,
            'pass rows' => TRUE
        )
    );
}

/**
 * Función que es llamada por el bulk operation y coge una solicitud metiendola 
 * en la cola para que la tienda lo pille
 */
function conectorSegeco1_sincronizarViales($node) {
    //Ahora cogemos los datos..        
    //Creamos el item para que cuando la tienda venga los coja.
    insert_item_into_buffer($node);
}
