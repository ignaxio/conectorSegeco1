<?php

/**
 * Función que mete en cola un vial cuando se crea una solicitud de genotipado 
 * o de contraste para que el conector  de la tienda lo pille
 */
function conectorSegeco1_node_presave($node) {
    if ($node->type == 'solicitud_genotipado') {
        //Se crea el título de la solicitud
        $node->title = conectorSegeco1_crearTituloSolicitud($node);
        //Se genera el super secreto dígito de control
        $node->field_perro_digito_de_control['und'][0]['value'] = conectorSegeco1_crearDigitoControl();                 
    }
//    dpm($node);
    if ($node->type == 'solicitud_genotipado' || $node->type == 'solicitud_de_contraste') {
        //Ahora cogemos los datos..        
        //Creamos el item para que cuando la tienda venga los coja.
        insert_item_into_buffer($node);
    }
//    node_save($node);
}

/**
 * Función que crea el título de la solicitud 
 * @param object solidcitud de genotipado $node
 * @return string
 */
function conectorSegeco1_crearTituloSolicitud($node) {
    $codigo_censo = "41740";

    if ($node->field_perro_peligroso['und'][0]['value'] == 1) {
        $Peligroso = "P";
        if ($node->field_perro_test_socializacion['und'][0]['value'] == 1) {
            $Bozal = "S";
        } else {
            $Bozal = "B";
        }
    } else {
        $Peligroso = "C";
        $Bozal = "S";
    }
    $codigo_censo = $codigo_censo . " " . $Peligroso . $Bozal;
    $numeroCenso = str_pad($node->field_perro_numero_serie['und'][0]['value'], 5, "0", STR_PAD_LEFT);
    $codigo_censo = $codigo_censo . " " . $numeroCenso;

    //Metemos el título de la solicitud
    return $codigo_censo;
}

/**
 * función que genera el dígito de control
 * @return String
 */
function conectorSegeco1_crearDigitoControl() {
    $str = "";
    $characters = range('A', 'Z');
    $max = count($characters) - 1;
    for ($i = 0; $i < 3; $i++) {
        $rand = mt_rand(0, $max);
        $str .= $characters[$rand];
    }
    return $str;
}
