// Copyright (C) 2012 Thomas Kjeldsen

var ShaderProgram = function (context, options)
{
  var m_Context = context;
  var m_Program;
  var m_VertexShader;
  var m_FragmentShader;
  var m_Prefix = "";
  var m_Ready = false;

  


  //var m_VertexShaderSourceID = vs_id;
  //var m_FragmentShaderSourceID = fs_id;

  var m_VertexShaderSource = null;
  var m_FragmentShaderSource = null;

  var m_VertexShaderUrl = null;
  var m_FragmentShaderUrl = null;

  var m_UniformLocations = {};
  var m_AttribLocations = {};

  if ( options.prefix !== undefined ) {
    m_Prefix = options.prefix;
  }
  if ( options.vs_url !== undefined ){
    m_VertexShaderUrl = options.vs_url;
  }
  if ( options.fs_url !== undefined ){
    m_FragmentShaderUrl = options.fs_url;
  }

  if ( options.vs_src !== undefined ){
    m_VertexShaderSource = options.vs_src;
  }
  if ( options.fs_src !== undefined ){
    m_FragmentShaderSource = options.fs_src;
  }
  

  m_Program = m_Context.createProgram();
  m_VertexShader = m_Context.createShader(m_Context.VERTEX_SHADER);
  m_FragmentShader = m_Context.createShader(m_Context.FRAGMENT_SHADER);


  if ( m_FragmentShaderSource && m_VertexShaderSource ) {
    CompileShaders();
  }
  else if ( m_FragmentShaderUrl && m_VertexShaderUrl ){

  var vsXhr = new XMLHttpRequest();
  vsXhr.open('GET', m_VertexShaderUrl);
  vsXhr.onload = function(){
    m_VertexShaderSource = this.responseText;
    var fsXhr = new XMLHttpRequest();
    fsXhr.open('GET', m_FragmentShaderUrl);
    fsXhr.onload = function(){
      m_FragmentShaderSource = this.responseText;
      
      CompileShaders();
    };
    fsXhr.send();
  };
  vsXhr.send();
  }
  else{
  }


  function CompileShaders()
  {

    if ( m_Prefix )
    {
      m_VertexShaderSource = m_Prefix + "\n" + m_VertexShaderSource;
      m_FragmentShaderSource = m_Prefix + "\n" + m_FragmentShaderSource;
    }
    m_Context.shaderSource(m_VertexShader, m_VertexShaderSource);
    m_Context.compileShader(m_VertexShader);

    if (m_Context.getShaderParameter(m_VertexShader, m_Context.COMPILE_STATUS) == false)
      console.log(m_Context.getShaderInfoLog(m_VertexShader));


    m_Context.shaderSource(m_FragmentShader, m_FragmentShaderSource);
    m_Context.compileShader(m_FragmentShader);

    if (m_Context.getShaderParameter(m_FragmentShader, m_Context.COMPILE_STATUS) == false)
      console.log(m_Context.getShaderInfoLog(m_FragmentShader));

    m_Context.attachShader(m_Program, m_VertexShader);
    m_Context.attachShader(m_Program, m_FragmentShader);

    m_Context.linkProgram(m_Program);
    if (m_Context.getProgramParameter(m_Program, m_Context.LINK_STATUS) == false)
      console.log(m_Context.getProgramInfoLog(m_Program));
    if (m_Context.getProgramParameter(m_Program, m_Context.VALIDATE_STATUS) == false)
      console.log(m_Context.getProgramInfoLog(m_Program));

    // Get uniforms
    m_Context.useProgram(m_Program);
    var numberofuniforms = m_Context.getProgramParameter(m_Program, m_Context.ACTIVE_UNIFORMS);

    var i;
    for ( i = 0; i < numberofuniforms; i++)
    {
      var activeuniform = m_Context.getActiveUniform(m_Program, i);
      m_UniformLocations[activeuniform.name] = m_Context.getUniformLocation(m_Program, activeuniform.name);
    }
    // Get attributes
    var numberofattribs = m_Context.getProgramParameter(m_Program, m_Context.ACTIVE_ATTRIBUTES);
    for ( i = 0; i < numberofattribs; i++)
    {
      var activeattrib = m_Context.getActiveAttrib(m_Program, i);
      m_AttribLocations[activeattrib.name] = m_Context.getAttribLocation(m_Program, activeattrib.name);
    }
    m_Ready = true;

  }

  
  this.GetUniformLocation = function(uniform)
  {
    return m_UniformLocations[uniform];
  };

  this.GetAttribLocation = function(attrib)
  {
    return m_AttribLocations[attrib];
  };

  this.GetProgram = function()
  {
    return m_Program;
  };
  this.GetReadyState = function()
  {
    return m_Ready;
  };

  this.RecompileShaders = function(prefix, callback)
  {
    if (prefix)
    {
      m_prefix = prefix;
    }

    m_Ready = false;
    //m_Context.useProgram(0);

  if ( m_FragmentShaderUrl && m_VertexShaderUrl ){
    var vsXhr = new XMLHttpRequest();
    vsXhr.open('GET', m_VertexShaderUrl);
    vsXhr.onload = function(){
      m_VertexShaderSource = this.responseText;
      var fsXhr = new XMLHttpRequest();
      fsXhr.open('GET', m_FragmentShaderUrl);
      fsXhr.onload = function(){
        m_FragmentShaderSource = this.responseText;


        m_Context.detachShader(m_Program, m_VertexShader);
        m_Context.detachShader(m_Program, m_FragmentShader);
        m_Context.deleteShader(m_VertexShader);
        m_Context.deleteShader(m_FragmentShader);
        m_Context.deleteProgram(m_Program);

        m_Program = m_Context.createProgram();
        m_VertexShader = m_Context.createShader(m_Context.VERTEX_SHADER);
        m_FragmentShader = m_Context.createShader(m_Context.FRAGMENT_SHADER);

        m_UniformLocations = {};
        m_AttribLocations = {};


        CompileShaders();

        if ( callback ){
          callback();
        }
      };
      fsXhr.send();
    };
    vsXhr.send();
  }
  else{
        CompileShaders();
  }

    return;
  };
};

module.exports = ShaderProgram;

// vim:  expandtab tabstop=2 shiftwidth=2 autoindent smartindent  
