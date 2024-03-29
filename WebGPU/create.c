﻿//Вычисляем координаты точек fermat spiral

const debugCount = %debugCountu;//Count of out debug values.

const verticesRowSize = 2u + debugCount;//в каждом ряду по две точки. Сюда можно добавить несколько отдадочных значений
const golden_angle = 137.5077640500378546463487;//137.508;//https://en.wikipedia.org/wiki/Golden_angle
const pi : f32 = 3.141592653589793;//Проблема с точностью. На самом деле получается 3.1415927410125732 ошибка уже в 8 знаке
const a = golden_angle * pi / 180.0;
const b = 90 * pi / 180.0;

const aNearRowLength = %aNearRowLengthu;//количество обнаруженных индексов вершин, ближайших к текущей вершине
//плюс индекс максимально удаленной вершины из массива aNear
//плюс максимальное количство индексов вершин, ближайших к текущей вершине
//плюс отладочная информация

//максимальное количество ребер вершины
const maxLength = (aNearRowLength
	- 1//место для количества обнаруженных индексов вершин, ближайших к текущей вершине
	- 1//место для индекса максимально удаленной вершины из массива aNear
	- debugCount//считается что место для отладочной информации в массиве индексов ближайших к текущей вершине вершин равно количеству мест для отладочной информации в массиве vertices с вершинами fermat spiral
) / (
	1 +//индекс ближайшей вершины
	1//расстояние между вершинами
);

struct Settings {
c: f32,//constant scaling factor. See Fermat's spiral https://en.wikipedia.org/wiki/Fermat%27s_spiral for details.
count : u32,//points count меньше общего количества точек для того что бы точки, расположенные на краю спирали имели ребра направленные к этим лишним точкам.
};
const settings = Settings(%c, %count);

struct ANear {
i: u32,//индекс вершины, ближайшей к текущей вершине
distance : f32,//distance between current vertice and nearest vertice.
}

//индексы вершин, которые ближе всего расположены к текущей вершине
//длинна этой структуры определена в aNearRowlength в файле fermatSpiral.js
struct VerticeANears {
length: u32,//количества обнаруженных индексов вершин, ближайших к текущей вершине
iMax : u32,//индекс максимально удаленной вершины из массива aNear
aNear : array<ANear, maxLength>,//индексы вершин, которые ближе всего расположены к текущей вершине
debug : array<u32, debugCount>,
}

//vertices. длинна строики в verticesRowlength
struct Vertices {
vertice: vec2<f32>,
aNear : VerticeANears,
edges: array<u32, maxLength>,//индексы ребер, которые имеют эту вершину
debug : array<f32, debugCount>
};
@group(0) @binding(0) var<storage, read_write> vertices : array <Vertices>;

//edges
//длинна этой структуры определена в edgesRowlength в файле fermatSpiral.js
struct EdgesItem {
i: u32,//индекс ребра
verticesIndices : array<u32, 2>,//индексы вершин ребра
debug : array<u32, debugCount>,
};
struct Edges {
length: u32,
indices : array<EdgesItem>,
}
@group(0) @binding(1) var<storage, read_write> edges : Edges;

@group(0) @binding(2) var<uniform> phase : u32;

@compute @workgroup_size(1)//, 1)
fn main(@builtin(global_invocation_id) global_id : vec3<u32>) {

	let i = global_id.x;//fermatSpiral vertice index

	switch (phase)
	{

		//Vertices
		case 0: {
			let angleInRadians = f32(i) * a - b;
			let radius = settings.c * sqrt(f32(i));
			vertices[i].vertice = vec2(radius * cos(angleInRadians), radius * sin(angleInRadians));

			//debug
			//vertices[i].edges = 789;// settings.c;
			vertices[i].debug[0] = 123;// settings.c;
			vertices[i].debug[1] = 456;

			break;
		}

		//aNear. See createEdgesAndFaces in FermatSpiral
		case 1: {
			var index = i * aNearRowLength;
			let vertice1 = vertices[i].vertice;//координаты вершины для которой будем искать ближайшие вершины

			//debug
			/*
			var indexDebug = index + aNearRowLength - debugCount;
			aNear[i].debug[0] = 123;// aNear[iMaxIndex];
			indexDebug++;
			aNear[indexDebug] = j;
			vertices[i * verticesRowSize + 2 + 0] = vecDistance;
			vertices[i * verticesRowSize + 2 + 1] = aNearDistance[aNearDistanceMaxIndex];
			*/

			for (var j = 0u; j < arrayLength(&vertices); j++)
			{
				if (i == j) { continue; }//не надо вычислять расстояние между одной и той же точкой
				let vertice2 = vertices[j].vertice;//Координаты текущей ближайшей вершины
				let vecDistance = distance(vertice1, vertice2);//расстояние между вершиной и текущей ближайшей вершиной

				if (vertices[i].aNear.length < maxLength)
				{

					//если массив бижайших вершии еще не заполнен
					//See "case 'add'" in "array.aNear = new Proxy" in "class Vector" in fermatSpiral.js

					vertices[i].aNear.aNear[vertices[i].aNear.length].i = j;//добавить индекс текущей ближайшей вершины
					vertices[i].aNear.aNear[vertices[i].aNear.length].distance = vecDistance;
					vertices[i].aNear.length++;

					getMax(i);

				} else {

					/*
					//debug
					if ((i == 2) && (j == 15)) {
						vertices[i].debug[0] = vecDistance;
						vertices[i].debug[1] = aNear[i].aNear[aNear[i].iMax].distance;
						aNear[i].debug[0] = aNear[i].iMax;
						aNear[i].debug[1] = j;
					}
					*/

					//Если максимально расстояние до ближайшей вершины больше растояние до текущей вершины, то заменить ближайшую вершину с максимальным расстоянием
					//See array.aNear = new Proxy add aNear in FermatSpiral
					if (vertices[i].aNear.aNear[vertices[i].aNear.iMax].distance > vecDistance) {

						vertices[i].aNear.aNear[vertices[i].aNear.iMax].i = j;//изменить индекс текущей ближайшей вершины
						vertices[i].aNear.aNear[vertices[i].aNear.iMax].distance = vecDistance;
						getMax(i);

					}

				}

			}

			//add edges
			//find '//add edges' comment in fermatSpiral.js
			let vertice1_aNear = vertices[i].aNear;//VerticeANears - индексы вершин, которые ближе всего расположены к текущей вершине
			for (var k = 0u; k < vertice1_aNear.length; k++) {
				let i1 = vertice1_aNear.aNear[k].i;
				var boDuplicate = false;

				for (var j = 0u; j < edges.length; j++){


				}
				if (boDuplicate || (i >= settings.count) || (i1 >= settings.count)) { continue; }

				edges.indices[edges.length].i = edges.length;

				//debug
				//edges.indices[edges.length].debug[0] = i1;
				//edges.indices[edges.length].debug[1] = debugCount;// maxLength;//settings.count;// paramsU32.count;

				edges.length++;
			}

			break;
		}

		//error message
		default: {
/*
			let zero = 0;
			let error = 1 / zero;
			aNear[i] = error;
*/
		}
	}
}

//Найти индекс максимально удаленной вершины из массива aNear
fn getMax(
	i : u32//for debug. fermatSpiral vertice index
) {

	var aNearItem = vertices[i].aNear;
	var iMax = aNearItem.iMax;
	for (var aNearIndex = 0u; aNearIndex < aNearItem.length; aNearIndex++) {

		if (aNearItem.aNear[iMax].distance < aNearItem.aNear[aNearIndex].distance) { iMax = aNearIndex; }

	}
	vertices[i].aNear.iMax = iMax;

}
