var group__http__server__struct =
[
    [ "cy_http_message_body_t", "structcy__http__message__body__t.html", [
      [ "data", "structcy__http__message__body__t.html#a3d07565cc43cf954e63ebaa5c2db1ca6", null ],
      [ "data_length", "structcy__http__message__body__t.html#a3cfde2375f1e55e21bd204a8f843f473", null ],
      [ "data_remaining", "structcy__http__message__body__t.html#afeaaa983c593be6dc2ba404ea086d44c", null ],
      [ "is_chunked_transfer", "structcy__http__message__body__t.html#a732a4fae8ab65d0d2b4e686c61402fbc", null ],
      [ "mime_type", "structcy__http__message__body__t.html#a38d892177ec43905ae3f8bd1107e1e31", null ],
      [ "request_type", "structcy__http__message__body__t.html#a63816ce209da9210ab93c3699ce775f9", null ]
    ] ],
    [ "cy_http_response_stream_t", "structcy__http__response__stream__t.html", [
      [ "tcp_stream", "structcy__http__response__stream__t.html#aa18641ca5f33f4a35f8c263e4c5a2b2a", null ],
      [ "chunked_transfer_enabled", "structcy__http__response__stream__t.html#add4a8cc936e3e5af9e3323ca03ddf716", null ]
    ] ],
    [ "cy_https_server_security_info_t", "structcy__https__server__security__info__t.html", [
      [ "private_key", "structcy__https__server__security__info__t.html#ab0df9038e60cccd0e8812c813c6b4c13", null ],
      [ "key_length", "structcy__https__server__security__info__t.html#ad9a1a0499abc77950cfde7aa19a9e3a2", null ],
      [ "certificate", "structcy__https__server__security__info__t.html#ac99aee7927d640ff6dfbe200dc44d8ae", null ],
      [ "certificate_length", "structcy__https__server__security__info__t.html#a3025afbe193cd4e514a711f09d3024e4", null ],
      [ "root_ca_certificate", "structcy__https__server__security__info__t.html#af85967438ec03bba793c57ea2f7d1b2c", null ],
      [ "root_ca_certificate_length", "structcy__https__server__security__info__t.html#a1b1bd39e4abc48f6789b001c7d67a554", null ]
    ] ],
    [ "cy_resource_dynamic_data_t", "structcy__resource__dynamic__data__t.html", [
      [ "resource_handler", "structcy__resource__dynamic__data__t.html#a2fa3f6203db2348184a4cd0409c1d6cb", null ],
      [ "arg", "structcy__resource__dynamic__data__t.html#a21189a067fa29eb836db706aa3553f2b", null ]
    ] ],
    [ "cy_resource_static_data_t", "structcy__resource__static__data__t.html", [
      [ "data", "structcy__resource__static__data__t.html#a898a2959797f5d333f70915e27c8f8bf", null ],
      [ "length", "structcy__resource__static__data__t.html#a288d94a277b56b226480d6d18e7787ce", null ]
    ] ],
    [ "EXPAND_AS_ENUMERATION", "group__http__server__struct.html#ga9570abf22d29227cd523da79c1e216eb", null ],
    [ "MIME_TABLE", "group__http__server__struct.html#ga517bd1687351d721dfa5f6ec4053493f", null ],
    [ "cy_http_server_t", "group__http__server__struct.html#ga047ac1c7063077c58da4cc991ad3b1bf", null ],
    [ "url_processor_t", "group__http__server__struct.html#ga6515ad09b10c358aa2d1d1efd4a4f2cd", null ],
    [ "cy_http_request_type_t", "group__http__server__struct.html#ga133e1438783bd7430e0e80b23518e743", [
      [ "CY_HTTP_REQUEST_GET", "group__http__server__struct.html#gga133e1438783bd7430e0e80b23518e743aae6efd1e64c25ff81cc5114051db036b", null ],
      [ "CY_HTTP_REQUEST_POST", "group__http__server__struct.html#gga133e1438783bd7430e0e80b23518e743a1c330cdb770148314e8e6c568b48acd9", null ],
      [ "CY_HTTP_REQUEST_PUT", "group__http__server__struct.html#gga133e1438783bd7430e0e80b23518e743ae6f844ee75f8cadd252bddfd3fde3532", null ],
      [ "CY_HTTP_REQUEST_UNDEFINED", "group__http__server__struct.html#gga133e1438783bd7430e0e80b23518e743ae033858556d98e39e1c0f624c83c0b4a", null ]
    ] ],
    [ "cy_http_cache_t", "group__http__server__struct.html#ga64b862dacedd67c3d74845c1a6038b89", [
      [ "CY_HTTP_CACHE_DISABLED", "group__http__server__struct.html#gga64b862dacedd67c3d74845c1a6038b89a21676ce46eff61339e33dded1daeaaea", null ],
      [ "CY_HTTP_CACHE_ENABLED", "group__http__server__struct.html#gga64b862dacedd67c3d74845c1a6038b89a1ba6f4e19b464a2b890040aa84c32a6b", null ]
    ] ],
    [ "cy_http_mime_type_t", "group__http__server__struct.html#gab6be7a35bfdbd62b763603d280dc4ee1", null ],
    [ "cy_http_status_codes_t", "group__http__server__struct.html#gad1b0486d4ab51b5b1969e54aae1e5edd", [
      [ "CY_HTTP_200_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5eddad3dbd9c89c618dff06785fdd77ec0419", null ],
      [ "CY_HTTP_204_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda5e39c01af97526ea6c9b9e4406f7f7d0", null ],
      [ "CY_HTTP_207_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda64594c9218c45ad4c7b424b819839587", null ],
      [ "CY_HTTP_301_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda31bb5770a3687bea226677fcbec762ef", null ],
      [ "CY_HTTP_400_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda19e795ef6a5cf0f638b8cd329aa6796a", null ],
      [ "CY_HTTP_403_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda09b0f7124d716a8b21c1d299f87180ab", null ],
      [ "CY_HTTP_404_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda4f45531dbee97b7a5dcc026a767eb977", null ],
      [ "CY_HTTP_405_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5eddaf0b401449ba5fcada5c49976e8d9b41a", null ],
      [ "CY_HTTP_406_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda56149d89c04690ca8aa577e123434930", null ],
      [ "CY_HTTP_412_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda887ccefc0440437c131f3639754cf3a1", null ],
      [ "CY_HTTP_415_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda1d7c7f7009c9a2030ebdd746f5b55f00", null ],
      [ "CY_HTTP_429_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5eddac667e393604e80ea50bfd22788911250", null ],
      [ "CY_HTTP_444_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda3e0ac33f9701d44a95cad9c5a30f4e05", null ],
      [ "CY_HTTP_470_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5edda56844ab35f128c4ca409ec704dcb3ac5", null ],
      [ "CY_HTTP_500_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5eddae099016f20b49bfd86f2b2282ab6858b", null ],
      [ "CY_HTTP_504_TYPE", "group__http__server__struct.html#ggad1b0486d4ab51b5b1969e54aae1e5eddad3c183b10c91de68efdf076efab73876", null ]
    ] ],
    [ "cy_url_resource_type", "group__http__server__struct.html#ga32d601085911dbd59b7ecd50a6b7f584", [
      [ "CY_STATIC_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584a16c98c125133981542d412c2449988a1", null ],
      [ "CY_DYNAMIC_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584ab61d536399015bfa53ede0e680aae9f7", null ],
      [ "CY_RESOURCE_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584a4e628b0071460eed61ea11b4d8b09087", null ],
      [ "CY_RAW_STATIC_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584a3a560fd0b3405a9114a1d50f721cd33b", null ],
      [ "CY_RAW_DYNAMIC_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584a6b8d1cd5fef8b7ffc82c06591b8e3bc1", null ],
      [ "CY_RAW_RESOURCE_URL_CONTENT", "group__http__server__struct.html#gga32d601085911dbd59b7ecd50a6b7f584a7a1b3a5cd969740f25d02edb5e2bafba", null ]
    ] ]
];