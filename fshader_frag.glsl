#version 330 core

in vec3 N;
in vec3 V;

uniform vec3 lightPos;
uniform vec3 coefficients;

out vec4 fragmentColor;

void main()
{
    
	// TODO 设置三维物体的材质属性
	float _tmp = 0.3;
	vec3 ambiColor = vec3(_tmp, _tmp, _tmp);
	_tmp = 0.6;
	vec3 diffColor = vec3(_tmp, _tmp, _tmp);
	_tmp = 1.0;
	vec3 specColor = vec3(_tmp, _tmp, _tmp);

	// TODO 计算N，L，V，R四个向量并归一化
	// vec3 N_norm = ...;
	// vec3 L_norm = ...;
	// vec3 V_norm = ...;
	// vec3 R_norm = ...;
	vec3 N_norm = normalize(N);
    vec3 L_norm = normalize(lightPos - V);
    vec3 V_norm = normalize(-V);
    vec3 R_norm = reflect(-L_norm, N_norm);


	// TODO 计算漫反射系数和镜面反射系数
	// float lambertian = ...;
	// float specular = ...;
	float lambertian = clamp(dot(L_norm, N_norm), 0.0, 1.0);
	float specular = clamp(dot(R_norm, V_norm), 0.0, 1.0);

	
	float shininess = 100.0;
		
	// TODO 计算最终每个片元的输出颜色
	// fragmentColor = ...;
	// fragmentColor = vec4(ambiColor + 
				   // diffColor * lambertian +
				   // specColor * pow(specular, 5.0), 1.0);
	// fragmentColor = vec4(specColor * pow(specular, 5.0), 1.0);
	_tmp = coefficients[0];
	vec4 ambient = vec4(_tmp * ambiColor, 1.0);
	_tmp = coefficients[1];
	vec4 diffuse = vec4(_tmp * diffColor * lambertian, 1.0);
	_tmp = coefficients[2];
	vec4 spec = vec4(_tmp * specColor * pow(specular, shininess), 1.0);
	fragmentColor = ambient + diffuse + spec;

}