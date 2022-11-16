//Вычисляем координаты точек fermat spiral

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
/*
//params
//Не получается в одну стркутуру засунуть данные разного типа
struct ParamsF32 {
c: f32,//constant scaling factor. See Fermat's spiral https://en.wikipedia.org/wiki/Fermat%27s_spiral for details.
}
@group(0) @binding(0) var<uniform> paramsF32 : ParamsF32;
struct ParamsU32 {
count: u32,//points count меньше общего количества точек для того что бы точки, расположенные на краю спирали имели ребра направленные к этим лишним точкам.
}
@group(0) @binding(1) var<uniform> paramsU32 : ParamsU32;
*/
//vertices. длинна строики в verticesRowlength
struct Vertices {
vertice: vec2<f32>,
//edges : f32,
edges: array<u32, maxLength>,//индексы ребер, которые имеют эту вершину
debug : array<f32, debugCount>
};
@group(0) @binding(0) var<storage, read_write> vertices : array <Vertices>;

//aNear
struct ANear {
i: u32,//индекс вершины, ближайшей к текущей вершине
distance: f32,//distance between current vertice and nearest vertice.
}

//длинна этой структуры определена в aNearRowlength в файле fermatSpiral.js
struct VerticeANears {
length: u32,//количества обнаруженных индексов вершин, ближайших к текущей вершине
iMax: u32,//индекс максимально удаленной вершины из массива aNear
aNear : array<ANear, maxLength>,//индексы вершин, которые ближе всего расположены к текущей вершине
debug: array<u32, debugCount>,
}
@group(0) @binding(1) var<storage, read_write> verticeANears : array<VerticeANears>;//<u32>;//индексы ближайших к текущей вершине вершин
//@group(0) @binding(2) var<storage, read_write> aNearDistance : array<f32>;//distance between current vertice and nearest vertices.

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
@group(0) @binding(2) var<storage, read_write> edges : Edges;

@group(0) @binding(3) var<uniform> phase : u32;

@compute @workgroup_size(1)//, 1)
fn main(@builtin(global_invocation_id) global_id : vec3<u32>) {

	let i = global_id.x;//fermatSpiral vertice index

	switch (phase)
	{

		//Vertices
		case 0: {
			let angleInRadians = f32(i) * a - b;
//			let radius = paramsF32.c * sqrt(f32(i));
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
//			let pointer1 = ptr<private, array<f32, 50>>;

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
				if (i == j) { continue; }
				let vertice2 = vertices[j].vertice;//Координаты текущей ближайшей вершины
				let vecDistance = distance(vertice1, vertice2);//расстояние между вершиной и текущей ближайшей вершиной

				if (verticeANears[i].length < maxLength) {

					verticeANears[i].aNear[verticeANears[i].length].i = j;//добавить индекс текущей ближайшей вершины
					verticeANears[i].aNear[verticeANears[i].length].distance = vecDistance;
					verticeANears[i].length++;

					//debug
					verticeANears[i].debug[0] = 678;
					verticeANears[i].debug[1] = 901;

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
					if (verticeANears[i].aNear[verticeANears[i].iMax].distance > vecDistance) {

						verticeANears[i].aNear[verticeANears[i].iMax].i = j;//изменить индекс текущей ближайшей вершины
						verticeANears[i].aNear[verticeANears[i].iMax].distance = vecDistance;
						getMax(i);

					}

				}

			}
			for (var k = 0u; k < verticeANears[i].length; k++) {
				let i1 = verticeANears[i].aNear[k].i;
				var boDuplicate = false;
//edges.length = 123;
//edges.indices[5] = 456;
				/*
				//debug
				if (k == 1) {
//					aNear[i].debug[0] = i1;// aNear[iMaxIndex];
					edges.indices[edges.length].debug[0] = 123;
				}
				*/

				for (var j = 0u; j < edges.length; j++){

					/*
					struct EdgesItem {
					i: u32,//индекс ребра
						verticesIndices : array<u32, 2>,//индексы вершин ребра
						debug : array<u32, debugCount>,
					};
					struct Edges {
					length: u32,
						indices : array<EdgesItem>,
					}
					edges[j] is edges.indices[j] ребро с индексом j
					edges[j][0] is edges.indices[j].verticesIndices[0] индекс нулевой вершины ребра
					if (
						((edges[j][0] === i) && (edges[j][1] === i1)) ||
						((edges[j][0] === i1) && (edges[j][1] === i))
						) {

						boDuplicate = true;
						break;

					}
					*/
					/*
					//debug
					edges.indices[edges.length].debug[0] = i1;
					edges.indices[edges.length].debug[1] = settings.count;// paramsU32.count;
					*/

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
	i : u32//fermatSpiral vertice index
) {

	var aNearItem = verticeANears[i];
	var iMax = aNearItem.iMax;
	for (var aNearIndex = 0u; aNearIndex < verticeANears[i].length; aNearIndex++) {

		if (aNearItem.aNear[iMax].distance < aNearItem.aNear[aNearIndex].distance) { iMax = aNearIndex; }
		/*
		//debug
		if (i == 1) {
			//vertices[i * verticesRowSize + 2 + 0] = aNearDistance[aNearDistanceIndex + iMax];
			//vertices[i * verticesRowSize + 2 + 1] = aNearDistance[aNearDistanceIndex + i];
			aNear[i].debug[0] = aNear[i].aNear[aNearIndex].i;
			aNear[i].debug[1] = aNear[i].iMax;
		}
		*/

	}
	verticeANears[i].iMax = iMax;

}
