<?php

function get_item_from_buffer() {
    $queue = DrupalQueue::get('BBIVialLleno');
    //si hay elementos los devolvemos.
    $item = $queue->claimItem();
    if ($item)
        return $item;
    else
        return FALSE;
}

function insert_item_into_buffer($node) {
    //vamos a ver de que ayntamiento estamos, en los segecos solo está su conector por lo que será el único.
    $ayto = NULL;
    $config = variable_get('codeserver_configs');
    foreach ($config as $conectorName => $server) {
        $ayto = str_replace('conector', '', $conectorName);
    }
    //Vamos a ver que tipo de vial es.
    $v = node_load($node->field_solicitud_codigovial['und'][0]['target_id']);

    $vial['tipoDeVial'] = $v->field_vial_tipo_de_vial['und'][0]['value'];
    $vial['titulo'] = isset($node->field_solicitud_codigovial['und'][0]['target_id']) ? conectorSegeco1_getNodeById($node->field_solicitud_codigovial['und'][0]['target_id']) : NULL;
    $vial['fechaVialLleno'] = date('d-m-Y h:i', $node->created);
    $vial['ayuntamiento'] = $ayto;
    $vial['veterinario'] = conectorSegeco1_getUserById($node->uid);
    $vial['tituloSolicitud'] = $node->title;
    $vial['tipoChapa'] = isset($node->field_perro_tipo_de_chapa['und'][0]['value']) ? $node->field_perro_tipo_de_chapa['und'][0]['value'] : NULL;
    $vial['nombrePerro'] = isset($node->field_perro_nombre_ejemplar['und'][0]['value']) ? $node->field_perro_nombre_ejemplar['und'][0]['value'] : NULL;
    $vial['digitoControl'] = isset($node->field_perro_digito_de_control['und'][0]['value']) ? $node->field_perro_digito_de_control['und'][0]['value'] : NULL;
    $vial['crearChapa'] = TRUE;
    $vial['tituloChapa'] = $vial['tituloSolicitud'] . ' ' . $vial['nombrePerro'] . ' ' . $vial['digitoControl'];

    $item = new stdClass();
    $item->data = $vial;
    $item->operaciones = array(2, 11, 100); //Insertar chapa, update vial y dejarlo en cola para segcan
    $queue = DrupalQueue::get('BBIVialLleno');
    $queue->createItem($item);
}

function remove_item_from_buffer($item_id) {
    $item = new stdClass();
    $item->item_id = $item_id;
    $queue = DrupalQueue::get('BBIVialLleno');
    $queue->deleteItem($item);
}

function insert_item_error_buffer($item) {
    $queue = DrupalQueue::get('BBIVialLleno_error');
    $queue->createItem($item);
}

function write_element_from_buffer($item) {
    //Aquí hay que hacer un switch para ir viendo las diferentes operaciones.
    //Las operaciones serán:
    //insertar vial, update vial, insertar chapa, eliminar viales de caja vacía
    $resultado = FALSE;
    $data = $item->data;
    foreach ($data->operaciones as $operacion) {
        $resultado = FALSE;
        switch ($operacion) {


            case 21://Insertar vial
                $vial = $data->vial;
                if (node_load(conectorSegeco1_getNodeByTitle($vial['titulo'])))
                    return false;
                $values = array(
                    'type' => 'vial',
                    'uid' => 1,
                    'status' => 1,
                    'comment' => 0,
                    'promote' => 0,
                );
                $entity = entity_create('node', $values);
                $ewrapper = entity_metadata_wrapper('node', $entity);
                $ewrapper->title->set($vial['titulo']);
                $ewrapper->field_vial_tipo_de_vial->set($vial['tipoDeVial']);
                //Si tiene veterinario lo metemos, si no, quiere decir que es un vial de heces para el ayto y  no tiene veterinario viene el valor 0
                if ($vial['veterinario']) {
                    $ewrapper->field_veterinario->set(conectorSegeco1_getUserByName($vial['veterinario']));
                }
                $ewrapper->save();
                if (node_load($ewrapper->getIdentifier()))
                    return true;
                break;
            case 22://Insertar chapa
                $chapa = $data->chapa;
                $values = array(
                    'type' => 'chapa',
                    'uid' => 1,
                    'status' => 1,
                    'comment' => 0,
                    'promote' => 0,
                );
                $entity = entity_create('node', $values);
                $ewrapper = entity_metadata_wrapper('node', $entity);
                $ewrapper->title->set($chapa['titulo']);
                $ewrapper->field_perro_tipo_de_chapa->set($chapa['tipo']);
                $ewrapper->field_perro_fecha_envio_chapa->set($chapa['fechaEnvio']);
                $ewrapper->save();
                if (node_load($ewrapper->getIdentifier()))
                    return true;
                break;

            case 31://Update vial
                $vial = $data->vial;
                //Cogemos el vial para poder acceder a la solicitud asociada
                if (!conectorSegeco1_getNodeByTitle($vial['titulo']))
                    return FALSE;
                $v = node_load(conectorSegeco1_getNodeByTitle($vial['titulo']));
                $idSolicitud = $v->field_vial_solicitud['und'][0]['target_id'];
                if (!$idSolicitud)
                    return FALSE;
                $s = node_load((int) $idSolicitud);

                //Vamos ha crear el pdf
//                $pdfTotal = crearPDF($idSolicitud, $vial['pdf1']);
                //Primero de todo vamos a grabar el genotipo...
                if ($vial['genotipo']) {
                    conectorSegeco1_setGen($v, $vial['genotipo']);
                }
                //accedo al estado de la solicitud y lo cambio
                $s->field_perro_estado['und'][0]['value'] = (int) $vial['resultado'];

                //Le meto la fecha de genotipado si la tiene...    
                if ($vial['fechaInforme']) {
                    $s->field_perro_fecha_genotipado['und'][0]['value'] = str_replace(" ", "T", $vial['fechaInforme']['value']);
                } else {
                    $s->field_perro_fecha_genotipado = array();
                }

                //Si es una solicitud de contraste..
                if (isset($s->field_contraste_vial_matching)) {
                    //Si tiene matching resultado = 1 positivo
                    if ($vial['matching']) {
                        $s->field_contraste_vial_matching['und'][0]['target_id'] = conectorSegeco1_getNodeByTitle($vial['matching']);
                        $s->field_perro_resultado['und'][0]['value'] = 1; //positivo
                    }
                    //Si el estado es 3 = no gen
                    if ($vial['resultado'] == 3) {
                        $s->field_perro_resultado['und'][0]['value'] = 0; //nulo
                    }
                    //Si el estado es 2 = ok y no tiene matching
                    if ($vial['resultado'] == 2 && !$vial['matching']) {
                        $s->field_perro_resultado['und'][0]['value'] = 2; //negativo
                    }
                }
                node_save($s);
                if ($s->field_perro_estado['und'][0]['value'] != $vial['resultado'])
                    return FALSE;
                return TRUE;

                break;
            case 32://Update chapa


                break;
            case 50://Eliminar viales vacios de una caja
                $tituloVial = $data->vial;
                $nidVial = conectorSegeco1_getNodeByTitle($tituloVial);
                //Nos aseguramos de que no tenga solicitud asociada.
                if(conectorSegeco2_checkVialVacio($nidVial)) {
                    node_delete($nidVial);                    
                    $resultado = TRUE;
                }
                break;
        }
    }
    return $resultado;
}

/**
 * Funcion que elimina los viales vacios de una caja cuando se cierra
 */
function conectorSegeco_eliminarVialesVaciosDeUnaCaja($viales) {
    $vialesVacios = array();

    //Vamos a coger los id de los viales que tienen aquí en lebrija
    foreach ($viales as $id => $titulo) {
        $vialesVacios[conectorSegeco1_getNodeByTitle($titulo)] = $titulo;
    }

    //Vamos a comprobar que no tengan solicitud asociada, si la tienen lo 
    //dejamos y no se elimina
    foreach ($vialesVacios as $idVial => $titulo) {
        if (conectorSegeco1_checkVialVacio($idVial)) {
            //Si está lleno lo eliminamos del array
            unset($vialesVacios[$idVial]);
        }
    }

    //Ahora ya podemos eliminar los viales seguros de que están vacios
    foreach ($vialesVacios as $idVial => $titulo) {
        node_delete($idVial);
    }
    return 'se han eliminado los viales correctamente';
}
